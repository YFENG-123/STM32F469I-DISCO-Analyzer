#include <gui/screen1_screen/Screen1View.hpp>
#include "main.h"

#include "arm_math.h"
#include "arm_const_structs.h" 
#define SAMPLE 256

extern uint32_t AdcConvertedValue[SAMPLE],AdcConvertedValueTemp[SAMPLE/2];
extern uint8_t AdcOverFlag;
extern ADC_HandleTypeDef hadc1,hadc2,hadc3;
extern TIM_HandleTypeDef htim4;
extern DAC_HandleTypeDef hdac;

float32_t freq,amplitude = 1.0,factor = 1.0,bias = 0.0,move = 0.0;
uint32_t daData12bit[100],daData12bitAmplitudeChanged[100];

int mode = 1,wave = 1;
int FFTSTOP = 0,ScanSTOP = 0;

arm_rfft_fast_instance_f32 S;

int16_t moveX=1,moveY=1;




Screen1View::Screen1View()
{
}
void Screen1View::setupScreen()
{
	Screen1ViewBase::setupScreen();
	
	//信号发生器初始波形
	float64_t temp;
	for(int i=0;i<100;i++)
	{
		temp = (float64_t)(amplitude*(4095.0*(sinf(2.0*3.1416*i/100.0f)+1)/2.0 - 4095.0/2) + 4095.0/2 + 2047.0*bias);
		if(temp > 4095)daData12bit[i] = 4095;
		else if(temp < 0)daData12bit[i] = 0;
		else daData12bit[i] = (uint32_t)temp;
	}
	htim4.Init.Period = 1;
	HAL_TIM_Base_Init(&htim4);
	HAL_TIM_Base_Start(&htim4);
	HAL_DAC_Start_DMA(&hdac,DAC_CHANNEL_1,(uint32_t*)daData12bit,100,DAC_ALIGN_12B_R);
}
void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}



//滑动手势回调函数
void Screen1View::handleGestureEvent(const GestureEvent& event)
{
	switch(swipeContainer1.getSelectedPage())
	{
		//示波器界面手势
		case 1:	
		{
			if(event.getType() == GestureEvent::SWIPE_HORIZONTAL && event.getVelocity() < -50)
			{
				swipeContainer1.goNextPage();
				Screen1View::function32();
			}
			else if(event.getType() == GestureEvent::SWIPE_HORIZONTAL && event.getVelocity() > 50)
			{
				swipeContainer1.goPreviousPage();
			}
				
			
			else if(event.getType() == GestureEvent::SWIPE_VERTICAL && event.getVelocity() > 25)
			{
				if(slideMenuScanBottom.getState() == SlideMenu::EXPANDED)slideMenuScanBottom.animateToState(SlideMenu::COLLAPSED);
				else slideMenuScanTop.animateToState(SlideMenu::EXPANDED);
			}

			
			else if(event.getType() == GestureEvent::SWIPE_VERTICAL && event.getVelocity() < -25)
			{
				if(slideMenuScanTop.getState() == SlideMenu::EXPANDED)slideMenuScanTop.animateToState(SlideMenu::COLLAPSED);
				else slideMenuScanBottom.animateToState(SlideMenu::EXPANDED);
			}
			
			break;
		}
		
		//频谱分析界面手势
		case 2:
		{
			if(event.getType() == GestureEvent::SWIPE_HORIZONTAL && event.getVelocity() > 50)
			{
				swipeContainer1.goPreviousPage();
				Screen1View::function50us();
			}
			else if(event.getType() == GestureEvent::SWIPE_VERTICAL && event.getVelocity() < -25)
			{
				slideMenuFFTBottom.animateToState(SlideMenu::EXPANDED);
			}
			else if(event.getType() == GestureEvent::SWIPE_VERTICAL && event.getVelocity() > 25)
			{
				slideMenuFFTBottom.animateToState(SlideMenu::COLLAPSED);
			}	
			break;
		}
	}
	//手势优化（暂时废弃）
	//if(event.getType() == GestureEvent::SWIPE_HORIZONTAL)moveX = event.getVelocity();
	//if(event.getType() == GestureEvent::SWIPE_VERTICAL)moveY = event.getVelocity();
}



//模式选择按钮点击回调
void Screen1View::function50us()
{
	dynamicGraph1.setVisible(true);
	dynamicGraph2.setVisible(false);
	dynamicGraph3.setVisible(false);
	dynamicGraph4.setVisible(false);
	dynamicGraph5.setVisible(false);
	dynamicGraph6.setVisible(false);
	dynamicGraph1.clear();
	mode=1;
}
void Screen1View::function100us()
{
	dynamicGraph1.setVisible(false);
	dynamicGraph2.setVisible(true);
	dynamicGraph3.setVisible(false);
	dynamicGraph4.setVisible(false);
	dynamicGraph5.setVisible(false);
	dynamicGraph6.setVisible(false);
	dynamicGraph2.clear();
	mode=2;
}
void Screen1View::function200us()
{
	dynamicGraph1.setVisible(false);
	dynamicGraph2.setVisible(false);
	dynamicGraph3.setVisible(true);
	dynamicGraph4.setVisible(false);
	dynamicGraph5.setVisible(false);
	dynamicGraph6.setVisible(false);
	dynamicGraph3.clear();
	mode=3;
}
void Screen1View::function32()
{
	dynamicGraph1.setVisible(false);
	dynamicGraph2.setVisible(false);
	dynamicGraph3.setVisible(false);
	dynamicGraph4.setVisible(true);
	dynamicGraph5.setVisible(false);
	dynamicGraph6.setVisible(false);
	dynamicGraph4.clear();
	mode=4;
	arm_rfft_fast_init_f32(&S, 64);
}
void Screen1View::function64()
{
	dynamicGraph1.setVisible(false);
	dynamicGraph2.setVisible(false);
	dynamicGraph3.setVisible(false);
	dynamicGraph4.setVisible(false);
	dynamicGraph5.setVisible(true);
	dynamicGraph6.setVisible(false);
	dynamicGraph5.clear();
	mode=5;
	arm_rfft_fast_init_f32(&S, 128);
}
void Screen1View::function128()
{
	dynamicGraph1.setVisible(false);
	dynamicGraph2.setVisible(false);
	dynamicGraph3.setVisible(false);
	dynamicGraph4.setVisible(false);
	dynamicGraph5.setVisible(false);
	dynamicGraph6.setVisible(true);
	dynamicGraph6.clear();
	mode=6;
	arm_rfft_fast_init_f32(&S, 256);
}

//波形偏置按钮点击回调
void Screen1View::functionUp()
{
	move += 124.0;//0.1*4096/3.3
	Unicode::snprintfFloat(textAreaBiasBuffer, 10, "%1.1f",move*3.300/4095.0);
}
void Screen1View::functionDown()
{
	move -= 124.0;//0.1*4096/3.3
	Unicode::snprintfFloat(textAreaBiasBuffer, 10, "%1.1f",move*3.300/4095.0);
}

//显示波形调整
void Screen1View::functionCompressVertical()
{
	factor -= 0.25;
}
void Screen1View::functionExpandVertical()
{
	factor += 0.25;
}
void Screen1View::functionCompressHorizontal()
{
	if(++mode > 3)mode = 3;
	switch(mode){
		case 1:Screen1View::function50us();break;
		case 2:Screen1View::function100us();break;
		case 3:Screen1View::function200us();break;
	}
}
void Screen1View::functionExpandHorizontal()
{
	if(--mode < 1)mode = 1;
	switch(mode){
		case 1:Screen1View::function50us();break;
		case 2:Screen1View::function100us();break;
		case 3:Screen1View::function200us();break;
	}
}


//启动/暂停开关点击回调
void Screen1View::functionToggleButtonFFT()
{
	if(toggleButtonFFT.getState())FFTSTOP = 0;
	else FFTSTOP = 1;
}
void Screen1View::functionToggleButtonScan()
{
	if(toggleButtonScan.getState())ScanSTOP = 0;
	else ScanSTOP = 1;
}

//输出波形类型单选按钮选择回调
void Screen1View::functionSin()
{
	HAL_DAC_Stop_DMA(&hdac,DAC_CHANNEL_1);
	HAL_TIM_Base_Stop(&htim4);
	float64_t temp;
	for(int i=0;i<100;i++)
	{
		temp = (float64_t)(amplitude*(4095.0f*(sinf(2.0f*3.1416*i/100.0f)+1)/2.0f - 4095.0/2) + 4095.0/2 + 2047.0*bias);
		if(temp > 4095)daData12bit[i] = 4095;
		else if(temp < 0)daData12bit[i] = 0;
		else daData12bit[i] = (uint32_t)temp;
	}
	HAL_TIM_Base_Init(&htim4);
	HAL_TIM_Base_Start(&htim4);
	HAL_DAC_Start_DMA(&hdac,DAC_CHANNEL_1, (uint32_t*)daData12bit,100,DAC_ALIGN_12B_R);
	wave = 1;
}
void Screen1View::functionTriangular()
{
	HAL_DAC_Stop_DMA(&hdac,DAC_CHANNEL_1);
	HAL_TIM_Base_Stop(&htim4);
	float64_t temp;
	for(int i=0;i<50;i++)
	{
		temp = (float64_t)(amplitude*(2*(4095.0*i/100.0f) - 4095.0/2) + 4095.0/2 + 2047.0*bias);
		if(temp > 4095)daData12bit[i] = 4095;
		else if(temp < 0)daData12bit[i] = 0;
		else daData12bit[i] = (uint32_t)temp;
	}
	for(int i=50;i<100;i++)
	{
		temp =(float64_t)(amplitude*(2*(4095.0-4095.0*i/100.0f) - 4095.0/2) + 4095.0/2 + 2047.0*bias);
		if(temp > 4095)daData12bit[i] = 4095;
		else if(temp < 0)daData12bit[i] = 0;
		else daData12bit[i] = (uint32_t)temp;
	}
	HAL_TIM_Base_Init(&htim4);
	HAL_TIM_Base_Start(&htim4);
	HAL_DAC_Start_DMA(&hdac,DAC_CHANNEL_1, (uint32_t*)daData12bit,100,DAC_ALIGN_12B_R);
	wave = 2;
}
void Screen1View::functionSquare()
{
	HAL_DAC_Stop_DMA(&hdac,DAC_CHANNEL_1);
	HAL_TIM_Base_Stop(&htim4);
	float64_t temp;
	for(int i=0;i<100;i++)
	{
		temp = (float64_t)(amplitude*(4095.0*(i/52) - 4095.0/2) + 4095.0/2 + 2047.0*bias);
		if(temp > 4095)daData12bit[i] = 4095;
		else if(temp < 0)daData12bit[i] = 0;
		else daData12bit[i] = (uint32_t)temp;
	}
	HAL_TIM_Base_Init(&htim4);
	HAL_TIM_Base_Start(&htim4);
	HAL_DAC_Start_DMA(&hdac,DAC_CHANNEL_1, (uint32_t*)daData12bit,100,DAC_ALIGN_12B_R);
	wave = 3;
}
void Screen1View::functionSawtooth()
{
	HAL_DAC_Stop_DMA(&hdac,DAC_CHANNEL_1);	
	HAL_TIM_Base_Stop(&htim4);
	float64_t temp;
	for(int i=0;i<100;i++)
	{
		temp = (float64_t)(amplitude*(4095.0*i/100.0f - 4095.0/2) + 4095.0/2 + 2047.0*bias);
		if(temp > 4095)daData12bit[i] = 4095;
		else if(temp < 0)daData12bit[i] = 0;
		else daData12bit[i] = (uint32_t)temp;
	}
	HAL_TIM_Base_Init(&htim4);
	HAL_TIM_Base_Start(&htim4);
	HAL_DAC_Start_DMA(&hdac,DAC_CHANNEL_1, (uint32_t*)daData12bit,100,DAC_ALIGN_12B_R);
	wave = 4;
}


//频率、幅值、偏置滚动条变化回调
void Screen1View::functionFreqChange(int value)
{
	HAL_TIM_Base_Stop(&htim4);//freq=500000.0f/(value+1)=90000000/(9*25*(Period+1))
	
	freq = 360000.0 / ((100 - value + 2)/2);
	Unicode::snprintfFloat(textCurrentFreqBuffer, 10, "%.0f", freq);
	textCurrentFreq.invalidate();

	uint32_t period = 101 - value;
	
	htim4.Init.Period = period;
	HAL_TIM_Base_Init(&htim4);
	HAL_TIM_Base_Start(&htim4);
	HAL_DAC_Start_DMA(&hdac,DAC_CHANNEL_1, (uint32_t*)daData12bit,100,DAC_ALIGN_12B_R);
}
void Screen1View::functionAmpChange(int value)
{
	amplitude=value/100.0f;
	Unicode::snprintfFloat(textCurrentAmpBuffer, 10, "%4.3f",3.300 * amplitude);
	textCurrentAmp.invalidate();
	switch(wave){
		case 1:Screen1View::functionSin();break;
		case 2:Screen1View::functionTriangular();break;
		case 3:Screen1View::functionSquare();break;
		case 4:Screen1View::functionSawtooth();break;
	}
}
void Screen1View::functionBiasChange(int value)
{
	bias = (value - 50.0)/50.0;
	Unicode::snprintfFloat(textCurrentBiasBuffer, 10, "%4.3f",3.300 * bias);
	textCurrentBias.invalidate();
	switch(wave){
		case 1:Screen1View::functionSin();break;
		case 2:Screen1View::functionTriangular();break;
		case 3:Screen1View::functionSquare();break;
		case 4:Screen1View::functionSawtooth();break;
	}
}



//主循环
void Screen1View::handleTickEvent()
{
	//手势优化（暂时废弃）
	/*if(moveX != 0 && moveY != 0){
		moveX = 0;
		moveY = 0;
		if(abs(moveX)>abs(moveY)){
			if(moveX > 0)slideMenuLeft.animateToState(SlideMenu::EXPANDED);
			else slideMenuRight.animateToState(SlideMenu::EXPANDED);
		}
		else{
			if(moveY > 0)slideMenuScanTop.animateToState(SlideMenu::EXPANDED);
			else slideMenuScanBottom.animateToState(SlideMenu::EXPANDED);
		}
	}*/
	
	static int tickCounter=0;
	if(++tickCounter == 50 && AdcOverFlag == 1)
	{

		tickCounter=0;
		
		//数据预处理
		uint32_t value_sum=0,max=0,min=4096,T=0,temp=AdcConvertedValue[0];
		int length_sum=0,count=0,last_max_index=0,riseFlag=0;
		for(int i=0;i<256;i++)
		{
			if(AdcConvertedValue[i]>max)max = AdcConvertedValue[i];//最大值搜寻
			if(AdcConvertedValue[i]<min)min = AdcConvertedValue[i];//最小值搜寻
			value_sum = value_sum + AdcConvertedValue[i];//累加求和
			
			//极值搜寻（暂时废弃）
			/*if(AdcConvertedValue[i]>temp)
			{
				riseFlag = 1;
				temp = AdcConvertedValue[i];
			}
			else if(riseFlag == 1)
			{
				riseFlag = 0;
				if(last_max_index!=0)
				{
					length_sum += i - last_max_index;
					count++;
				}
				last_max_index = i;
			}*/
		}
		
		uint32_t value_diff = max - min;//峰峰值计算
		float32_t value_average = float32_t(value_sum) / 256.0;//均值计算
		//uint32_t cyc = length_sum/count;//周期计算
		
		Unicode::snprintfFloat(textAreaVmaxBuffer, 10, "%4.3f",max*3.3/4096 + bias);
		Unicode::snprintfFloat(textAreaVminBuffer, 10, "%4.3f",min*3.3/4096 + bias);
		Unicode::snprintfFloat(textAreaVppBuffer, 10, "%4.3f",(max-min)*3.3/4096);
		Unicode::snprintfFloat(textAreaVarvBuffer, 10, "%4.3f",value_average*3.3/4096);
		//Unicode::snprintfFloat(textAreaTBuffer, 10, "%4.3f",cyc);
		//Unicode::snprintfFloat(textAreafBuffer, 10, "%4.3f",cyc);

		
		//FFT计算预计算
		static float32_t testInput_f32[SAMPLE],testOutput_f32[SAMPLE*2],testOutputMag_f32[SAMPLE/2 - 1];//定义FFT输入、过程、结果储存变量
		for(int i=0;i<SAMPLE;i++)testInput_f32[i] = (AdcConvertedValue[i] - value_average )*3.300/4096;
		
		//显示预计算
		for(int i=0;i<SAMPLE;i++)AdcConvertedValue[i] = (factor*(AdcConvertedValue[i] + move - value_average) + value_average);
		
		switch(mode)
		{
			case 1:{
				if(ScanSTOP)break;
				for(int i=0;i<64;i++)dynamicGraph1.addDataPoint(float(AdcConvertedValue[i]*3.300/4096));
				break;
			}
			case 2:{
				if(ScanSTOP)break;
				for(int i=0;i<128;i++)dynamicGraph2.addDataPoint(float(AdcConvertedValue[i]*3.300/4096));
				break;
			}
			case 3:{
				if(ScanSTOP)break;
				for(int i=0;i<256;i++)dynamicGraph3.addDataPoint(float(AdcConvertedValue[i]*3.300/4096));
				break;
			}
			case 4: {
				if(FFTSTOP)break;

				//FFT
				int FFTLEN = 64;
				arm_rfft_fast_f32(&S, testInput_f32, testOutput_f32, 0);  
				testOutputMag_f32[0] = fabsf(testOutput_f32[0]);     
				testOutputMag_f32[FFTLEN/2] = fabsf(testOutput_f32[1]);    
				arm_cmplx_mag_f32(testOutput_f32 + 2, testOutputMag_f32 + 1, (FFTLEN/2)-1);  
				
				//绘制频谱图
				float index = 0;
				dynamicGraph4.addDataPoint(testOutputMag_f32[0] / FFTLEN);
				for (int max=0,i=1; i < FFTLEN/2; i++)
				{
					dynamicGraph4.addDataPoint(testOutputMag_f32[i]/(FFTLEN/2));
					if(max<testOutputMag_f32[i]){
						max=testOutputMag_f32[i];
						index = i;
					}
				}
				dynamicGraph4.addDataPoint(testOutputMag_f32[FFTLEN/2] / FFTLEN);

				float mainFreqHz = index * (2500000  / float(FFTLEN));
				Unicode::snprintfFloat(textFFTFreqBuffer, 10, "%.0f", mainFreqHz);
				Unicode::snprintfFloat(textFFTTBuffer, 10, "%.4f", (1.0/mainFreqHz)*1000.0);
				break;
			}
			case 5:{
				if(FFTSTOP)break;
				
				//FFT
				int FFTLEN = 128;
				arm_rfft_fast_f32(&S, testInput_f32, testOutput_f32, 0);
				testOutputMag_f32[0] = fabsf(testOutput_f32[0]);
				testOutputMag_f32[FFTLEN/2] = fabsf(testOutput_f32[1]); 
				arm_cmplx_mag_f32(testOutput_f32 + 2, testOutputMag_f32 + 1, (FFTLEN/2)-1); 
				
				//绘制频谱图
				float index = 0;
				dynamicGraph5.addDataPoint(testOutputMag_f32[0] / FFTLEN);
				for(int max=0,i=1;i<FFTLEN/2;i++)
				{
					dynamicGraph5.addDataPoint(testOutputMag_f32[i]/(FFTLEN/2));
					if(max<testOutputMag_f32[i]){
						max=testOutputMag_f32[i];
						index = i;
					}
				}
				dynamicGraph5.addDataPoint(testOutputMag_f32[FFTLEN/2] / FFTLEN);	
				
				float mainFreqHz = index * (2500000  / float(FFTLEN));
				Unicode::snprintfFloat(textFFTFreqBuffer, 10, "%.0f", mainFreqHz);
				Unicode::snprintfFloat(textFFTTBuffer, 10, "%.4f", (1.0/mainFreqHz)*1000.0);
				break;
			}
			case 6:{
				if(FFTSTOP)break;
				
				//FFT
				int FFTLEN = 256;
				arm_rfft_fast_f32(&S, testInput_f32, testOutput_f32, 0);
				testOutputMag_f32[0] = fabsf(testOutput_f32[0]); 
				testOutputMag_f32[FFTLEN/2] = fabsf(testOutput_f32[1]); 
				arm_cmplx_mag_f32(testOutput_f32 + 2,testOutputMag_f32 + 1,(FFTLEN/2)-1);
				
				//绘制频谱图
				float index = 0;
				dynamicGraph6.addDataPoint(testOutputMag_f32[0] / FFTLEN);
				for(int max=0,i=1;i<FFTLEN/2;i++)
				{
					dynamicGraph6.addDataPoint(testOutputMag_f32[i]/(FFTLEN/2));
					if(max<testOutputMag_f32[i]){
						max=testOutputMag_f32[i];
						index = i;
					}
				}
				dynamicGraph6.addDataPoint(testOutputMag_f32[FFTLEN/2] / FFTLEN);
				
				float mainFreqHz = index * (2500000 / float(FFTLEN));
				Unicode::snprintfFloat(textFFTFreqBuffer, 10, "%.0f", mainFreqHz);
				Unicode::snprintfFloat(textFFTTBuffer, 10, "%.4f", (1.0/mainFreqHz)*1000.0);
				break;
			}
		}
		HAL_ADC_Start_DMA(&hadc1,AdcConvertedValue,SAMPLE);
		
		//多ADC共同采样
		//HAL_ADC_Start(&hadc2);
		//HAL_ADC_Start(&hadc3);
		//HAL_ADCEx_MultiModeStart_DMA(&hadc1,AdcConvertedValue,SAMPLE/2);
		//HAL_ADCEx_MultiModeStart_DMA(&hadc2,AdcConvertedValue,256);
		//HAL_ADCEx_MultiModeStart_DMA(&hadc3,AdcConvertedValue,256);
	}
}
