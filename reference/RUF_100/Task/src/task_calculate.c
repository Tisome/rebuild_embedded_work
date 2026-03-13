#include "task_manager.h"
#include "calculate.h"
#include "data.h"
#include "dac.h"
#include "tim.h"
#include "elog.h"
#include "bsp_gp22_handler.h"
#include "data.h"
#include "output.h"

double time_now;
double time_past = 0;

//瞬时流量
double g_flow_rate;
//累计流量
double g_flow_info; //当前流量统计


KalmanFilter kf_flow_rate;//flow_rate 卡尔曼滤波


double time_del;  //系统误差

double filtered_flow_rate;  //滤波后的流速
double flow_rate_raw; 
double filetered_flow_rate_offset; //修正后的流速

double flow_ave;  //窗口中流速的平均值


static TaskHandle_t task_calculate_Handle = NULL;/* 创建任务句柄 */



//空载判断相关变量
int SQ_count = 0;
//输出相关
int pulse_num = 0;
int pulse_f = 100;
void parameter_init();
void npn_pnp_output(double frequncy);
void set_output_current_ma(double flow_rate);
void SQ_check(void);

int flag_over = 0;
int flag_reverse = 0;
// 按 total_unit 计算脉冲当量
static inline double get_conv_per_flow_rate(void) 
{
    switch (g_parameters.total_unit) 
    {
        case 0:  return g_parameters.k_factor;
        case 1:  return g_parameters.k_factor / 1000.0;
        case 2:  return g_parameters.k_factor / 1000000.0;
        default: return g_parameters.k_factor;
    }
}
//核心计算：基于 no_kf_flow_rate[30] 完成去异常、平均、卡尔曼、流量/电流/累计/脉冲 
static inline void recompute_from_window(double no_kf_flow_rate[30])         // 累脉冲阈值)    // 微秒
{
   static double  Residual_flow_rate = 0;
    double buf[30];
    // 1) 拷贝 + IQR 去异常
    for (int i = 0; i < 30; ++i) buf[i] = no_kf_flow_rate[i];
    remove_outliers_IQR(buf);  // 置0异常值

    // 2) 平均 + 卡尔曼 + 缩放 + 绝对值
    flow_ave = kf_data_average(buf, 30);
    flow_rate_raw = Kalman_Update(&kf_flow_rate, flow_ave);
    filtered_flow_rate = flow_rate_raw * g_parameters.scale_factor;
    if((filtered_flow_rate > (g_parameters.offset* g_parameters.scale_factor + 5)) || 
    (filtered_flow_rate < (g_parameters.offset* g_parameters.scale_factor - 5)))
    {
        flag_over = 1;
		filtered_flow_rate = g_parameters.offset * g_parameters.scale_factor;
	}
    else{
        flag_over = 0;
    }
    //小流量切除
    if((filtered_flow_rate> g_parameters.offset* g_parameters.scale_factor  && filtered_flow_rate < (g_parameters.zero_cut + g_parameters.offset* g_parameters.scale_factor))        ||
	   (filtered_flow_rate<g_parameters.offset* g_parameters.scale_factor  && filtered_flow_rate > (-g_parameters.zero_cut + g_parameters.offset* g_parameters.scale_factor))        ||
       (0.94 > g_GP22_data.empty_tube))
    {
		filtered_flow_rate = g_parameters.offset * g_parameters.scale_factor;
	}

    // 3) 流量计算
    filetered_flow_rate_offset = filtered_flow_rate;
    double flow_mid_calculate = flow_calculate(filetered_flow_rate_offset - g_parameters.offset * g_parameters.scale_factor) * 3600.0;
    //流速反转
    if (flow_mid_calculate < 0) 
    {
        flow_mid_calculate = -flow_mid_calculate;
        flag_reverse = 1;
    }
    else
    {
        flag_reverse = 0;
    }
    
    g_flow_rate   = flow_mid_calculate; // 本次计算得到的体积流量


    // 4) 输出电流
	if(g_parameters.output_mode == 0)
    {
        set_output_current_ma(g_flow_rate);
    }
    // 5) 累计体积（按毫秒时间增量）
    time_now  = xTaskGetTickCount();
    if(!flag_reverse)
    {
        g_flow_info += g_flow_rate * (time_now - time_past) / 3600000.0; // ΔV = Q(h^-1) * Δt(h)
    }
    time_past = time_now;

    // 6) 脉冲当量与输出
    const double conv = get_conv_per_flow_rate();
	pulse_f = (g_flow_rate/3600)/(conv) +1;
	if(g_parameters.output_mode >= 1)
    {
	    npn_pnp_output(pulse_f/2);
    }
}
static void task_calculate(void *params)
{
    parameter_init();
    vTaskDelay(pdMS_TO_TICKS(100)); // 等待系统稳定
    double no_kf_flow_rate[30];
	Kalman_Init(&kf_flow_rate, g_flow_rate, 15.0, 0.005, 0.1);  //流速滤波初始化
	
	
	TickType_t xLastWakeTime = xTaskGetTickCount();

	emit_init();
    time1_4ms_delay_init();
    flowout_config();
    wdt_monitor_task_register("calculate_task", 800, &calculate_task_id);
    wdt_monitor_task_enable(calculate_task_id);  
	while (1)
	{ 
        SQ_check();
        double t1c[30], t2c[30];
		time_del = g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].time_del + 
        g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].e_factor* g_parameters.wall_thickmess;
		
        const int idx = sample_count % 30;

        // 情况 A：窗口满 30 个样本时，整窗重算
        if (sample_count == 33) 
        {
            for (int i = 0; i < 30; ++i) 
            {
                t1c[i] = g_tof_up[i];
                t2c[i] = g_tof_down[i];
            }
        // 先把 no_kf_flow_rate[30] 用整窗计算一次（统一入口）
            for (int i = 0; i < 30; ++i) 
            {
                const double t1s = (t1c[i] - time_del) * 1e-6;  // -> 秒
                const double t2s = (t2c[i] - time_del) * 1e-6;
                no_kf_flow_rate[i] = velocity_calculate_no_wave_speed(t1s, t2s);
            }
        // 整窗一次性输出更新
        recompute_from_window(no_kf_flow_rate);
        }
        // 情况 B：窗口未满 or 正在滚动，用增量写入 + 每 5 次重算
        else if(sample_count > 33)
        {   
            // 仅当当前点有效时才写入
            double t1s = (g_tof_up[idx] - time_del) * 1e-6;
            double t2s = (g_tof_down[idx] - time_del) * 1e-6;
            no_kf_flow_rate[idx] = velocity_calculate_no_wave_speed(t1s, t2s);

            // 每 5 次样本触发一次整窗统计与输出
            if ((sample_count % 5) == 0) 
            {
                recompute_from_window(no_kf_flow_rate);
            }
        }
        wdt_monitor_task_feed(calculate_task_id);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(8));
	}
} 

TaskHandle_t* get_task_calculate_Handle(void)
{
    return &task_calculate_Handle;
}
void do_create_calculate_task(void)
{	
	BaseType_t xReturn = pdPASS;						/* 定义一个创建信息返回值，默认为pdPASS */
	/* 创建AppTaskCreate任务 */
	xReturn = xTaskCreate(task_calculate, "task_calculate", 1024, NULL, 15, &task_calculate_Handle);      
}
void parameter_init()
{
    Parameters_t default_pipparameters =
    {
        .outer_diameter =          12.0,
        .wall_thickmess =           1.0,
        .scale_factor   =          1.00,
        .pipe_type      =   PIPE_PLATIC,
        .fluid_type     =  LIQUID_WATER,
        .cos            =      0.913545,
        .sin            =      0.406738,
        .is_saved       =             0,
        .lower_range    =          1.00,
        .upper_range    =         20.00,
        .flow_rate_unit =             0,
        .total_unit     =             0,
        .modbus_addr    =             1,
        .k_factor       =             5,
        .output_mode    =             0,
        .b_select_mode  =             0,        
        .offset         =             0,
        .fixed_error_bias =           0,
        .language       =             1,
    };
    LoadParameters(&g_parameters);

    if(g_parameters.is_saved != 1)
    {
        SaveParameters(&default_pipparameters);
    }
    LoadParameters(&g_parameters);
    // //材质相关的初始的设置
    // double tmp_cal   = (g_parameters.outer_diameter)/5.0;
    // double mask_mod  =                  tmp_cal * 0x0080;
    // switch(g_parameters.b_select_mode)
    // {
    // case 0:
    //     g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask +=0; 
    //     break;
    // case 1:
    //     g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask +=mask_mod; 
    //     break;
    // case 2:
    //     g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask -=mask_mod; 
    //     break;
    // case 3:
    //     g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask -=mask_mod; 
    // break;
    // }
    dac_set_fir_vol(g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].dac_fire);
    gp22_app_modify_reg_attr(GP22_CONF_REG2, 
                g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask,
                                                                 g_parameters.pipe_type);	
                
}

void  npn_pnp_output(double frequncy)
{
    int newPsc = 10000 / frequncy - 1;
    if (2 >= frequncy)
    {
        timer_disable(TIMER0);
        return;
    }
    // 2. 写入新参数到预装载寄存器（此时不生效，仅暂存）
    timer_parameter_struct timer_initpara;
    timer_initpara.prescaler         = newPsc > 65535 ? 65535 : newPsc;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 12000 - 1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER0,&timer_initpara);

    timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_0,6000);
    timer_enable(TIMER0);
}

// 设置输出电流值 (4.0 - 20.0 mA)
static void set_output_current(float current_ma)
{
    // 电流范围限制
    if (current_ma < 4.0f) current_ma = 4.0f;
    if (current_ma > 20.0f) current_ma = 20.0f;
    
    // 计算所需电压 (基于XTR111特性)
    // V_in = (I_out - 4mA) * 5kΩ / 1000
    float voltage = (current_ma - 4.0f) * 5.0f;
    
    
    // 计算PWM占空比 (假设MCU工作电压3.3V) 4000对应百分之50
    // int ma_cycle = current_ma*288;
        int ma_cycle = current_ma*477+508;
    
    
    // 更新PWM占空比
    timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_0,ma_cycle);
    timer_enable(TIMER0);
}

// 设置输出电流值 (4.0 - 20.0 mA)
static void set_output_current_ma(double flow_rate)
{   double tmp_rate;    
    switch (g_parameters.flow_rate_unit)
    {
        case 0: // m3 或 m3/h
            tmp_rate = flow_rate;
            break;
        case 1: // L 或 L/min 
            tmp_rate = flow_rate * (1000.0 / 60.0); // m3/h -> L/min
            break;
        case 2: // mL 或 mL/min
            tmp_rate = flow_rate * (1000000.0 / 60.0); // m3/h -> mL/min
            break;
        case 3: // L/h 
            tmp_rate = flow_rate * 1000; // m3/h -> L/h
            break;
    }
    float ma = 16 *((tmp_rate - g_parameters.lower_range)    / 
    (g_parameters.upper_range -g_parameters.lower_range)) + 4;
    set_output_current(ma);
}

void SQ_check(void)
{
    static unsigned int  SQ_check_num = 0;
    static float flag_mode = false;
    if(        (g_GP22_data.tof_up - g_GP22_data.tof_down)> 1.5    || 
             (g_GP22_data.tof_up - g_GP22_data.tof_down)< -0.05    ||
                                        (g_GP22_data.tof_up>150)   ||
                                      (g_GP22_data.tof_down>150)   ||
                   (g_GP22_data.tof_up  == g_GP22_data.tof_down))  
    {
        SQ_count++;
    }
    SQ_check_num++;
    if(SQ_check_num % 375 == 0)
    {
        g_GP22_data.empty_tube = 1.0 - SQ_count/375.0;
        uint16_t tmp_cal = (g_parameters.outer_diameter - 2*g_parameters.wall_thickmess)/5;
        uint16_t mask_mod   = (uint16_t)tmp_cal * 0x0080;
        double mask_fixed_err = (g_parameters.outer_diameter - 2*g_parameters.wall_thickmess)/5.0;
        //异常值大于百分之30，探测屏蔽区间
        if (SQ_count > 23)
        {
            //根据内径动态修改屏蔽区间
            if (0 == g_parameters.b_select_mode)
            {
                if (flag_mode == false)
                {
                    flag_mode = true;
                }
                else
                {
                    for(int i =0;i<3;i++)
                    {
                        // g_materialConfigs[GetMaterialConfigIndex()][i].mask +=mask_mod;
                        // g_materialConfigs[GetMaterialConfigIndex()][i].time_del +=mask_fixed_err;  
                    }
                    //log_d("g_materialConfigs_mod = 0x%x",g_materialConfigs[GetMaterialConfigIndex()][1].mask);
                    // gp22_app_modify_reg_attr(GP22_CONF_REG2, g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask,
                    //                                                      g_parameters.pipe_type);	
                    flag_mode = false;
                    g_parameters.b_select_mode++;
                }
                // log_d("Sq = %d,change b = 0x%x",SQ_count,g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask);
            }
            else if (1 == g_parameters.b_select_mode)
            {
                for(int i =0;i<3;i++)
                {
                    // g_materialConfigs[GetMaterialConfigIndex()][i].mask -=(mask_mod * 2); 
                    // g_materialConfigs[GetMaterialConfigIndex()][i].time_del -=mask_fixed_err * 2; 
                }
                // gp22_app_modify_reg_attr(GP22_CONF_REG2, g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask,
                //                                                          g_parameters.pipe_type);	
                g_parameters.b_select_mode++;
                // log_d("Sq = %d,change b = 0x%x",SQ_count,g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask);
            }
            else if (2 == g_parameters.b_select_mode)
            {
                for(int i =0;i<3;i++)
                {
                    // g_materialConfigs[GetMaterialConfigIndex()][i].mask +=(mask_mod * 1); 
                    // g_materialConfigs[GetMaterialConfigIndex()][i].time_del +=mask_fixed_err; 
                }
                // gp22_app_modify_reg_attr(GP22_CONF_REG2, g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask,
                //                                                          g_parameters.pipe_type);	
                g_parameters.b_select_mode = 0;
            }
        }
        else
        {
            g_GP22_data.empty_tube = 1.0 - SQ_count/375.0;
            // if ((3 == g_parameters.b_select_mode || 2 == g_parameters.b_select_mode) && SQ_count < 18)
            // {
            //     //根据内径动态修改屏蔽区间
            //     for(int i =0;i<3;i++)
            //     {
            //         g_materialConfigs[GetMaterialConfigIndex()][i].mask +=mask_mod; 
            //     }
            //     gp22_app_modify_reg_attr(GP22_CONF_REG2, g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask,
            //                                                                  g_parameters.pipe_type);	
            //     g_parameters.b_select_mode = 0;
            //     // log_d("Sq = %d,change b = 0x%x",SQ_count,g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask);
            // }
        }
        SQ_count = 0;
        SQ_check_num = 0;
    }
    if(g_GP22_data.empty_tube < 0)
    {
        g_GP22_data.empty_tube = 0;
    }
}