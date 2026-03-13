#include "math.h"
               
double velocity_calculate(double delta_time);

double flow_calculate(double velocity);
#define ACC_SAMPLES 1
typedef struct {
    double x;   // 系统状态估计
    double P;   // 估计误差协方差
    double Q;   // 过程噪声方差
    double R;   // 观测噪声方差
} KalmanFilter;
double calculate_filtered_average(KalmanFilter kf_timeout ,double data[ACC_SAMPLES]);
void Kalman_Init(KalmanFilter *kf, double init_x, double init_P, double Q, double R);
double Kalman_Update(KalmanFilter *kf, double measurement);
void Kalman_Init_process(KalmanFilter *kf, double time[]);
double velocity_calculate2(double delta_time);
void Kalman_Compare_Init(KalmanFilter *kf_origin, KalmanFilter *kf_process,double delta_time_alive);
void Kalman_Init_process(KalmanFilter *kf, double time[]);

void selectionSort(double arr[], int n);

double kf_data_average(double data[], int num);
void remove_outliers_IQR(double data[]);
//声明计算函数
double velocity_calculate_no_wave_speed(double filtered_timeout1, 
			double filtered_timeout2); 