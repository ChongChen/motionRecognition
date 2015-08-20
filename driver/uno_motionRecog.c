/*
 * uno_motionRecog.c
 *
 *	Created on: 2015.08.03
 *   Edited on:
 *		Author: Chen Chong
 *
 */


#include <math.h>
#include <string.h>
#include <pthread.h>
#include "uno_motionRecog.h"

#include "uno_api.h"
//
//gyr xyz bias的初始值
static float gyrx_bias = -0.01568079;
static float gyry_bias = 0.02071555;
static float gyrz_bias = 0.0186753;
//acc energy bias 的初始值
static float acc_energy_bias = 1.0;

//detectVibration函数中的算法参数
//振动的时间长度阈值
static int T_period = 10;
//信息熵阈值
static float T_entrophy = 2.8;


//recognizeState函数中的算法参数
//判断动静的阈值
static float T_acc_energy_slow = 0.02;
static float T_gyr_energy_slow = 0.01;
//与旋转相关的能量阈值
static float T_gyr_energy_sum_rotate_lower = 1.5;
static float T_gyr_energy_sum_rotate_upper = 12;
//与振动相关的能量阈值
static float T_gyr_energy_sum_vibration = 3;
//与移动相关的能量阈值
static float T_gyr_energy_sum_acc = 0.25;


static MOTION_STATE_CB cb;
static pthread_t motion_thread;



//设置算法参数阈值
void moiton_parameters_set(float parameters[]){
	T_period = parameters[0];
	T_entrophy = parameters[1];
	T_acc_energy_slow = parameters[2];
	T_gyr_energy_slow = parameters[3];
	T_gyr_energy_sum_rotate_lower = parameters[4];
	T_gyr_energy_sum_rotate_upper = parameters[5];
	T_gyr_energy_sum_vibration = parameters[6];
	T_gyr_energy_sum_acc = parameters[7];
}


float max(float x, float y){
	if (x > y)
		return x;
	else
		return y;
}


//arr_开头的都是工具函数，完成一些向量计算
float arr_sum(float x[], int N){
	float res = 0;
	for (int i = 0; i < N; i++)
		res += x[i];

	return res;
}


float arr_max(float x[], int N){
	float max_v = -1000000;
	for (int i = 0; i < N; i++)
		if (x[i] > max_v)
			max_v = x[i];

	return max_v;
}


float arr_min(float x[], int N){
	float min_v = 10000000;
	for (int i = 0; i < N; i++)
		if (x[i] < min_v)
			min_v = x[i];

	return min_v;
}


float arr_avg(float x[], int N){
	return arr_sum(x, N)/N;
}

float arr_std(float x[], int N){
	float avg = arr_avg(x, N);
	float t = 0;
	for (int i = 0; i < N; i++){
		t += (x[i]-avg)*(x[i]-avg);
	}
	return sqrt(t/N);
}

void arr_add(float x[], float f, int N){
	for (int i = 0; i < N; i++)
		x[i] += f;
}

void arr_sub(float x[], float f, int N){
	arr_add(x, -f, N);
}




/*更新gyr_xyz以及acc energy的bias
 *input: arr_gyrX, arr_gyrY, arr_gyrZ, energy_acc
 *output: gyrX_bias, gyrY_bias, gyrz_bias, acc_energy_bias
 */
void updateBias(float arr_gyrX[], float arr_gyrY[], float arr_gyrZ[], float energy_acc[], int N){
	gyrx_bias = arr_sum(arr_gyrX, N)/N;
	gyry_bias = arr_sum(arr_gyrY, N)/N;
	gyrz_bias = arr_sum(arr_gyrZ, N)/N;

	acc_energy_bias = arr_sum(energy_acc, N)/N;
	
//	printf("%f, %f, %f, %f\n\n", gyrx_bias, gyry_bias, gyrz_bias, acc_energy_bias);
}

/*消除几个输入中的bias
 */
void correctBias(float arr_gyrX[], float arr_gyrY[], float arr_gyrZ[], float energy_acc[], int N){
	arr_sub(arr_gyrX, gyrx_bias, N);
	arr_sub(arr_gyrY, gyry_bias, N);
	arr_sub(arr_gyrZ, gyrz_bias, N);
	for (int i = 0; i < N; i++){
		float t = energy_acc[i] - acc_energy_bias;
		energy_acc[i] = sqrt(t*t);
	}
}

/*利用三轴传感器来计算能量
 * input: x, y, z
 * output: energy
 */
void getEnergy(float x[], float y[], float z[], float energy[], int N){
	for (int i = 0; i < N; i++){
		float tmp = sqrt(x[i]*x[i] + y[i]*y[i] + z[i]*z[i]);
		energy[i] = tmp;
	}
}

/*计算过零率
 */
int calZeroCrossRate(float x[], int N){
	int zeroCrossRate = 0;
	for (int i = 0; i < N-1; i++){
		if (x[i]*x[i+1] < 0)
			zeroCrossRate ++;
	}

	return zeroCrossRate;
}


/*计算信息熵
 */
float calEntrophy(float energy[], int N){
	float Z = arr_sum(energy, N);
	float entrophy = 0;

	for (int i = 0; i < N; i++){
		float tmp = energy[i]/Z;
		entrophy -= tmp*log(tmp);
	}

	return entrophy;
}


/*检测可能的振动状态
 * input: energy_gyr, energy_acc
 * output: 1(可能存在振动)， 0反之
 */
int detectVibration(float energy_gyr[], float energy_acc[], int N){
	float T_upper = 0.02;
	float T_lower = 0.01;

//	int T_period = 10;
//	float T_entrophy = 2.8;

	float energy_max = arr_max(energy_gyr, N);

	int start_ind = -1;
	int end_ind = N-1;

	int max_period = 0;
	int num_period = 0;

	for (int i = 0; i < N; i++)
		if (start_ind == -1 && energy_gyr[i] >= T_upper)
			start_ind = i;
		else if (start_ind != -1 && (energy_gyr[i] < T_lower || i == N-1)){
			end_ind = i;
			int tmp = end_ind - start_ind;
			start_ind = -1;
			num_period += 1;
			if (tmp > max_period)
				max_period = tmp;
		}

	float entrophy_gyr = calEntrophy(energy_gyr, N);
	float entrophy_acc = calEntrophy(energy_acc, N);

	if ((max_period < T_period) && (entrophy_acc < T_entrophy || entrophy_gyr < T_entrophy))
		return 1;
	else
		return 0;
}


/*这些参数为svm模型的参数
 * 这里分类模型主要用来却别振动和移动
 */
float features_mean[] = {0.000952000973704 , 0.194286318888 , 0.0385421548696 , 0.0469895260649 , 2.7227848987 , 0.00269014390511 , 0.199455724819 , 0.0472600002731 , 0.0501533674823 , 2.77176581178};

float features_std[] = {0.00173738012652 , 0.218467389142 , 0.0586856225518 , 0.0582037726523 , 0.4353989121 ,0.00280160435942 , 0.181247415458 , 0.0569260882929 , 0.0526250891454 , 0.430318042765 };

float vibration_coef[] = {-0.901028005951 , 0.240717631089 , 0.215008926282 , 0.400052482044 , -0.240335062946, -0.456934238853 , 0.69209487438 , -1.61818759692 , -0.598541043596 , -0.437675523147};

float vibration_intercept = -0.81884858;


/*为分类模型获取特征
 *
 */
const int features_num = 10;
float features[features_num];
void extractFeatures(float energy_acc[], float energy_gyr[], int N){

	features[0] = arr_min(energy_acc, N);
	features[1] = arr_max(energy_acc, N);
	features[2] = arr_avg(energy_acc, N);
	features[3] = arr_std(energy_acc, N);
	features[4] = calEntrophy(energy_acc, N);

	features[5] = arr_min(energy_gyr, N);
	features[6] = arr_max(energy_gyr, N);
	features[7] = arr_avg(energy_gyr, N);
	features[8] = arr_std(energy_gyr, N);
	features[9] = calEntrophy(energy_gyr, N);

	for (int i = 0; i < features_num; i++){
		features[i] = (features[i]-features_mean[i])/features_std[i];
	}

}


/*分类器识别算法
 *
 */
int classifyVibration(float energy_acc[], float energy_gyr[], int N){
	extractFeatures(energy_acc, energy_gyr, N);

	float tmp = 0;
	for (int i = 0; i < features_num; i++)
		tmp += features[i]*vibration_coef[i];
	tmp += vibration_intercept;

	if (tmp > 0)
		return 1;
	else
		return 0;
}




/*识别一段时间片（长度为interval）是属于哪个状态
 *input: arr_accX, arr_accY, arr_accZ, arr_gyrX, arr_gyrY, arr_gyrZ, energy_acc, energy_gyr
 *output: 0(静止)，1(振动)，2(移动)，3(旋转)
 */
int recognizeState(float arr_accX[], float arr_accY[], float arr_accZ[], float arr_gyrX[], float arr_gyrY[], float arr_gyrZ[], float energy_acc[], float energy_gyr[], int interval){
	
//	float T_acc_energy_slow = 0.02;
//	float T_gyr_energy_slow = 0.01;

//	float T_gyr_energy_sum_rotate_lower = 1.5;
//	float T_gyr_energy_sum_rotate_upper = 12;

//	float T_gyr_energy_sum_vibration = 3;
//	float T_gyr_energy_sum_acc = 0.25;


	int zeroCrossRate_gyrX = calZeroCrossRate(arr_gyrX, interval);
	int zeroCrossRate_gyrY = calZeroCrossRate(arr_gyrY, interval);
	int zeroCrossRate_gyrZ = calZeroCrossRate(arr_gyrZ, interval);


	int isRotatingPotential = 0;
	if (zeroCrossRate_gyrX == 0 || zeroCrossRate_gyrY == 0 || zeroCrossRate_gyrZ == 0)
		isRotatingPotential = 1;

	float acc_seg_max = arr_max(energy_acc, interval);
	float gyr_seg_max = arr_max(energy_gyr, interval);

	float energy_gyr_seg_sum = arr_sum(energy_gyr, interval);
	//default state is still
	int default_state = 0;

	int isVibrationPotential = detectVibration(energy_gyr, energy_acc, interval);
	//classifyVibration(energy_acc, energy_gyr, interval);

	if (gyr_seg_max > T_gyr_energy_slow || acc_seg_max > T_acc_energy_slow){
		// rotating
		if ((isRotatingPotential == 1 && energy_gyr_seg_sum > T_gyr_energy_sum_rotate_lower) || energy_gyr_seg_sum > T_gyr_energy_sum_rotate_upper)
			default_state = 3;
		// vibration
		else if (isVibrationPotential == 1 && energy_gyr_seg_sum <= T_gyr_energy_sum_vibration && classifyVibration(energy_acc, energy_gyr, interval))	
			default_state = 1;
		
		// moving
		else if (energy_gyr_seg_sum > T_gyr_energy_sum_acc)
			default_state = 2;
	}

	return default_state;
}


/*调用unoSDK，进行实时识别
 * 提供一个运动状态和相应的能量大小给回调函数
 *input: para, 该参数是滑动窗口的步长，跟识别速度有关
 *
 */
void recognizeMotion(void* para){

	int stepLen = *(int*)para;

	int ret;
	ret = uno_ctrl_init(NULL, NULL);
	if (ret == 0){
		printf("init success\n");
	}
	else{
		printf("uno_ctral_init error, error number:%d\n", ret);
	}



	UNO_IMU_DATA_T data_acc;
	UNO_IMU_DATA_T data_gyr;

	const int frag_len = 30;
	float frag_accX_orig[frag_len];
	float frag_accY_orig[frag_len];
	float frag_accZ_orig[frag_len];

	float frag_gyrX_orig[frag_len];
	float frag_gyrY_orig[frag_len];
	float frag_gyrZ_orig[frag_len];

	// copy on orig sig
	float frag_accX[frag_len];
	float frag_accY[frag_len];
	float frag_accZ[frag_len];

	float frag_gyrX[frag_len];
	float frag_gyrY[frag_len];
	float frag_gyrZ[frag_len];

	float energy_acc[frag_len];
	float energy_gyr[frag_len];

	// 跳过最初的一些点
	int M = frag_len;
	while(M--){
		uno_imu_get_acc(&(data_acc.x), &(data_acc.y), &(data_acc.z));
		uno_imu_get_gyr(&(data_gyr.x), &(data_gyr.y), &(data_gyr.z));
	}


	// 自适应动静阈值的设置
	M = 5;
	float energy_gyr_for_init = 0;
	while(M--){
		for (int i = 0; i < frag_len; i++){
			uno_imu_get_gyr(&(data_gyr.x), &(data_gyr.y), &(data_gyr.z));
			uno_imu_get_acc(&(data_acc.x), &(data_acc.y), &(data_acc.z));

			frag_gyrX_orig[i] = data_gyr.x;
			frag_gyrY_orig[i] = data_gyr.y;
			frag_gyrZ_orig[i] = data_gyr.z;

			frag_accX_orig[i] = data_acc.x;
			frag_accY_orig[i] = data_acc.y;
			frag_accZ_orig[i] = data_acc.z;
		}
		getEnergy(frag_gyrX_orig, frag_gyrY_orig, frag_gyrZ_orig, energy_gyr, frag_len);
		//printf("%f", arr_max(energy_gyr, frag_len));

		float t = arr_max(energy_gyr, frag_len);

		//printf("test:%f\n", max(energy_gyr_for_init, arr_max(energy_gyr, frag_len)));
		if (t > energy_gyr_for_init)
			energy_gyr_for_init = t;

		printf("init:%f\n", energy_gyr_for_init);
	}
	float T_energy_gyr_init = energy_gyr_for_init*2;
	printf("Init gyr energy threshold: %f\n\n", T_energy_gyr_init);

	while(1){
		//printf("step length: %d\n\n", stepLen);
		//数组向左移位
		int t_ind = frag_len - stepLen;
		for (int i = 0; i < t_ind; i++){
			int j = i + stepLen;
			frag_accX_orig[i] = frag_accX_orig[j];
			frag_accY_orig[i] = frag_accY_orig[j];
			frag_accZ_orig[i] = frag_accZ_orig[j];

			frag_gyrX_orig[i] = frag_gyrX_orig[j];
			frag_gyrY_orig[i] = frag_gyrY_orig[j];
			frag_gyrZ_orig[i] = frag_gyrZ_orig[j];

		}
		//采集新数据
		for (int i = t_ind; i < frag_len; i++){
			uno_imu_get_acc(&(data_acc.x), &(data_acc.y), &(data_acc.z));
			uno_imu_get_gyr(&(data_gyr.x), &(data_gyr.y), &(data_gyr.z));

			frag_accX_orig[i] = data_acc.x;
			frag_accY_orig[i] = data_acc.y;
			frag_accZ_orig[i] = data_acc.z;

			frag_gyrX_orig[i] = data_gyr.x;
			frag_gyrY_orig[i] = data_gyr.y;
			frag_gyrZ_orig[i] = data_gyr.z;
		}
		//复制数据,因为frag_acc后续计算过程中数据会发生变化
		//因此要保留frag_acc_orig
		for (int i = 0; i < frag_len; i++) {
			frag_accX[i] = frag_accX_orig[i];
			frag_accY[i] = frag_accY_orig[i];
			frag_accZ[i] = frag_accZ_orig[i];

			frag_gyrX[i] = frag_gyrX_orig[i];
			frag_gyrY[i] = frag_gyrY_orig[i];
			frag_gyrZ[i] = frag_gyrZ_orig[i];
			
		}


		getEnergy(frag_gyrX, frag_gyrY, frag_gyrZ, energy_gyr, frag_len);
		getEnergy(frag_accX, frag_accY, frag_accZ, energy_acc, frag_len);

		float energy_gyr_max = arr_max(energy_gyr, frag_len);
		//printf("Biased gyr energy max:%f\n", energy_gyr_max);

		
		if (energy_gyr_max < T_energy_gyr_init){
			printf("update...\n");
			updateBias(frag_gyrX, frag_gyrY, frag_gyrZ, energy_acc, frag_len);
		}
		correctBias(frag_gyrX, frag_gyrY, frag_gyrZ, energy_acc, frag_len);

		getEnergy(frag_gyrX, frag_gyrY, frag_gyrZ, energy_gyr, frag_len);
		
		
		
		float gyr_energy_sum = arr_sum(energy_gyr, frag_len);
		
		int state = recognizeState(frag_accX, frag_accY, frag_accZ, frag_gyrX, frag_gyrY, frag_gyrZ, energy_acc, energy_gyr, frag_len);

		//当有事件发生时调用回调函数
		if (cb && state > 0)
			cb(state, gyr_energy_sum);


		/*	
		switch (state){
		//	case 0: printf("Still, "); break;
			case 1: printf("Vibrating, "); break;
			case 2: printf("Moving, "); break;
			case 3: printf("Rotating, "); break;
		}
		printf("Energy sum: %f\n\n\n", gyr_energy_sum);
		*/
	}	
	
}

/*主函数入口，进行参数初始化
 *利用了双线程和回调函数，当通过传感器识别到了运动状态时，就调用一次回调函数
 *
 *
 */
void motionRecogInit(MOTION_STATE_CB motion_cb, int stepLen){
	cb = motion_cb;
	//recognizeMotion((void*)&stepLen);
	
	float parameters[] = {10, 2.8, 0.02, 0.01, 1.5, 12, 3, 0.25};
	moiton_parameters_set(parameters);

	int ret = pthread_create(&motion_thread, NULL, recognizeMotion,(void*)&stepLen);
	if (ret != 0){
		printf("create motion recognition thread error.\n\n");
	}
	else
		printf("create motion recognition thread success.\n\n");
	
}
