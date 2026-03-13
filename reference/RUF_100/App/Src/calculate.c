#include "calculate.h"
#include "main.h"
#include "string.h"
#include <stdlib.h>
#include "data.h"
#include <math.h>
#include "elog.h"

extern Parameters_t g_parameters;
extern Parameters_t default_pipparameters;

//double g_cos_theta = 0.913545; //????cos?μ
const double sin_theta = 0.445229; //????sin?μ 
double PIPE_DISTANCE = 0.0274;  //外径，后续可改位  PIPE_DISTANCE = g_pipparameters.outer_diameter;
double WAVE_SPEED = 1500.0;   //?é?ê?ù?è
const double pi = 3.1415926;
extern bool flow_rate_no[30];

double PIPE_S_four;  //pi * PIPE_DISTANCE * PIPE_DISTANCE


double velocity_calculate(double delta_time)
{
	double velocity;
  	PIPE_DISTANCE = g_parameters.outer_diameter - 2*g_parameters.wall_thickmess;
	double velocity_up_sqrt = PIPE_DISTANCE*PIPE_DISTANCE + delta_time*delta_time*WAVE_SPEED*WAVE_SPEED*g_parameters.cos*g_parameters.cos;
	double velocity_up = sqrt(velocity_up_sqrt) - PIPE_DISTANCE;
	double velocity_down = delta_time*g_parameters.cos*g_parameters.cos;
	if(velocity_down != 0){
		velocity = velocity_up / velocity_down;
	}
	else 
	{
		velocity = 0;
	}
		
	return velocity ;
}


/*   速度计算函数，没有声速，公式中将声速消去了  */
double velocity_calculate_no_wave_speed(double filtered_timeout1, double filtered_timeout2)
{
	double velocity;
	double velocity_son;
	double velocity_mom;
	double g_pipparameters_sin = 0.406738;
	double g_pipparameters_cos = 0.913545;
	
	//计算管道内径
	PIPE_DISTANCE =  (g_parameters.outer_diameter - 2*g_parameters.wall_thickmess) / 1000;

	/*  W型  */
	// if(g_parameters.pipe_type == 1)
	// {
	// 	//分子为：D * 时间差
	// 	velocity_son = 2 * PIPE_DISTANCE * (filtered_timeout1 - filtered_timeout2);  
	// 	//分母为：t1*t2*sin*cos
	// 	velocity_mom = filtered_timeout1 * filtered_timeout2 *  0.3636468;
	// }
	// /*  V型  */
	// else
	// {
		//分子为：D * 时间差
		velocity_son = PIPE_DISTANCE * (filtered_timeout1 - filtered_timeout2);  
		//分母为：t1*t2*sin*cos
		velocity_mom = filtered_timeout1 * filtered_timeout2 * 0.37157346621;
		//DN50
		// velocity_mom = filtered_timeout1 * filtered_timeout2 * 0.2926;
	// }
	
	//计算速度，分母不能过小
	velocity = (velocity_mom == 0) ? 0 : (velocity_son / velocity_mom);

	return velocity;
}



double velocity_calculate2(double delta_time)
{
	double flow;
	flow = WAVE_SPEED*WAVE_SPEED*delta_time/(4*PIPE_DISTANCE*g_parameters.cos);
	return flow-0.036;
}
double flow_calculate(double velocity)
{
	double flow;
	PIPE_S_four = pi * PIPE_DISTANCE * PIPE_DISTANCE;
	flow = velocity*PIPE_S_four/4;
	return flow;
}
char* doubleToString(double d,int i) {
    static char str[50]; // 静态缓冲区（注意线程安全问题）
    if (i < 0) i = 0;
    snprintf(str, sizeof(str), "%.*f",i, d); // 保留小数点后三位
    return str;
}

void Kalman_Init(KalmanFilter *kf, double init_x, double init_P, double Q, double R) {
    kf->x = init_x;
    kf->P = init_P;
    kf->Q = Q;
    kf->R = R;
}
// 单次卡尔曼滤波更新
double Kalman_Update(KalmanFilter *kf, double measurement) {
    /* 预测阶段 */
    double P_pred = kf->P + kf->Q;  // 预测协方差
    
    /* 更新阶段 */
    double K = P_pred / (P_pred + kf->R);  // 卡尔曼增益
    kf->x = kf->x + K * (measurement - kf->x);  // 状态更新
    kf->P = (1.0 - K) * P_pred;  // 协方差更新
    
    return kf->x;
}
//过程中卡尔曼滤波更新
void Kalman_Init_process(KalmanFilter *kf, double time[]) {
    Kalman_Init(kf, time[0], 15,0.001,4);
		for(int i = 1; i<5 ; i++)
			Kalman_Update(kf,time[i]);
}
//过程中比较卡尔曼滤波是否超值
void Kalman_Compare_Init(KalmanFilter *kf_origin, KalmanFilter *kf_process,double delta_time_alive)
{
	double delta_kf_time = kf_origin->x - kf_process->x;
	const double delta_max = 1*1e-3; //如果超过这个值就会进行重置
	if(delta_kf_time > delta_max && delta_kf_time < -delta_max)
	{
		Kalman_Init(kf_origin,delta_time_alive,15,0.001,4);
	}
}

//平均计算
double kf_data_average(double data[], int num)
{
	double sum = 0;
	int n = num;
	
	double ave;
	for(int i = 0; i<n;i++)
	{
		if(0 == data[i])
			num--;
		else 
			sum += data[i];
	}
	ave = sum / num;
	return ave;
}




//IQR,四分位法剔除异常值
void remove_outliers_IQR(double data[]) {
	
    int n = 30;  // 数组大小
//    qsort(data, n, sizeof(double), compare);  //排序数据
	
		selectionSort(data, n);
		
    
		//计算Q1和Q3
    int Q1_index = n / 4;  //第一四分位
    int Q3_index = 3 * n / 4;  //第三四分位
    double Q1 = data[Q1_index];
    double Q3 = data[Q3_index];
	
		//计算IQR	
    double IQR = Q3 - Q1;
	
		//printf("Q1:%lf,%d,Q3:%lf,%d,  IQR:%lf\n", Q1, Q1_index, Q3, Q3_index,IQR);
    
		//定义上下限
    double lower_bound = Q1 - 1.5 * IQR;  //
    double upper_bound = Q3 + 1.5 * IQR;
		
		//printf("low:%lf  ,up:%lf\n", lower_bound, upper_bound);

		//遍历，剔除异常值
    for (int i = 0; i < n; i++) {
        if (data[i] < lower_bound || data[i] > upper_bound) {
              data[i] = 0;
        }
    }
		//for(int i=0;i<n;i++)printf("%lf, %d\n",data[i], i);

}


//选择排序
void selectionSort(double arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int minIndex = i;
        
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[minIndex]) {
                minIndex = j;
            }
        }
        
        if (minIndex != i) {
            double temp = arr[i];
            arr[i] = arr[minIndex];
            arr[minIndex] = temp;
        }
    }
}
