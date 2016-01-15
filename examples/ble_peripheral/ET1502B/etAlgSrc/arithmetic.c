#include "ET1502B.h"
#include "arithmetic.h"
#if WALK_STEP_RECORD_ARITHMETIC
#include <string.h>
#include "common_arithmetic.h"
#include "et_debug.h"

#if USE_ARITHMETIC_FROM == XIAOMI_ARITHMETIC
void Arithmetic_Set_YAxis_Min_Threshold(unsigned short data) 
{
	//g_yAxisDaltMinThreshold = data;
}


/*
	1.均值滤波算法
*/
static void fir_process(unsigned short *src, 
						unsigned short *dist,
						unsigned short length, 
						unsigned char fir_smooth_interval)
{	
	unsigned int sum = 0; 
	unsigned short i;
	unsigned short  j = 0;
	unsigned char k = 0;
	
	//for(k=0;k<2;k++)
	{
		for(i = 0;i < length - fir_smooth_interval;i++)
		{	
			sum = 0;
			for(j = 0;j < fir_smooth_interval;j++)
			{
				sum += src[i+j];  
			} 
			dist[i] = sum/fir_smooth_interval;
		}
		for(i = length - fir_smooth_interval; i < length; i++)
		{	
			sum = 0;
			for(j = 0;j < fir_smooth_interval;j++)
			{
				sum += src[i-j];  
			}
			dist[i] = sum/fir_smooth_interval;
		}
	}

}

/*
	1.求导算法
*/
static void derivative(unsigned short length, unsigned short *src, signed short *dist)
{
	unsigned short i = 0;
	if(length > 2)
	{
		*dist = *(src+1) - *src;
		*(dist + length - 1) = *(src + length - 1) - *(src + length - 2);

		for(i = 1; i < length - 2; i++)
			*(dist + i) = *(src + i + 1) - *(src + i - 1);
	}
	else
	{
		for(i = 0; i < length; i++)
			*(dist + i) = *(src + i);
	}
	return;
}

/*
	1.找波峰
*/
static unsigned short find_peaks(unsigned short *fir_src, 
								signed short* derivative_src,
								unsigned short length,
								unsigned short amp_threshold, 
								xy_data_array* peaks_array)
{
	unsigned short j = 0, peakCount = 0, start_pos = 0;
	unsigned short smooth_width = SMOOTH_WIDTH;
	if(length <= smooth_width)
		smooth_width = length/2;

	if(length <= 2)
	{
		return length;
	}
	
	
    if(smooth_width > 1)
		start_pos = 2*(smooth_width/2) - 1;
	else
		start_pos = 1;
	
    for (j = start_pos-1 ; j < length - smooth_width - 1 ;j++)
    {
        if((*(derivative_src+j) >= 0) && (*(derivative_src+j+1) < 0))
        {
            if(*(fir_src + j) > start_pos)   
            {
				*(peaks_array->x_array + peakCount) = j;
				*(peaks_array->y_array + peakCount) = *(fir_src + j);
				peakCount++;
            } 
        }
    }
	peaks_array->num = peakCount;
	return peakCount;
}

/*
	1.找波谷
*/

static unsigned short find_valleys( unsigned short *fir_src,
									signed short* derivative_src,
									unsigned short length,
									unsigned short amp_threshold, 
									xy_data_array* valleys_array )
{
	unsigned short j = 0, valley_count = 0, start_pos = 0;
	unsigned short smooth_width = SMOOTH_WIDTH;
	if(length <= smooth_width)
		smooth_width = length/2;

	if(length <= 2)
	{
		return length;
	}
	
    if(smooth_width > 1)
		start_pos = 2*(smooth_width/2) - 1;
	else
		start_pos = 1;

    for (j = start_pos-1 ; j < length - smooth_width - 1 ;j++)
    {
        if((*(derivative_src+j) < 0) && (*(derivative_src+j+1) >= 0))
        {       
            if(*(fir_src + j) > amp_threshold)   
            {                 
				*(valleys_array->x_array + valley_count) = j;
				*(valleys_array->y_array + valley_count) = *(fir_src + j);
				valley_count++;
                    
            }
        }
    }
	valleys_array->num = valley_count;
	return valley_count;
}

/*
	1.合并波谷波峰
*/
static void peaks_valleys_merge(xy_data_array* peaks_array, xy_data_array* valleys_array, xy_data_array* merge_array)
{
	unsigned short i = 0, count = 0, peak_pos = 0, valley_pos = 0;
	
	memset(merge_array->x_array,0,BLE_ARTH_BUF_SIZE);
	memset(merge_array->y_array,0,BLE_ARTH_BUF_SIZE);
	merge_array->num = 0;
	
	while(i < (peaks_array->num + valleys_array->num))
	{
		if((peak_pos < peaks_array->num) && (valley_pos < valleys_array->num))
		{
			if(*(peaks_array->x_array + peak_pos) < *(valleys_array->x_array + valley_pos))
			{
				*(merge_array->x_array + count) = *(peaks_array->x_array + peak_pos);
				*(merge_array->y_array + count) = *(peaks_array->y_array + peak_pos);
				peak_pos++;
				count++;
				i++;
			}
			else if(*(peaks_array->x_array + peak_pos) > *(valleys_array->x_array + valley_pos))
			{
				*(merge_array->x_array + count) = *(valleys_array->x_array + valley_pos);
				*(merge_array->y_array + count) = *(valleys_array->y_array + valley_pos);
				valley_pos++;
				count++;
				i++;
			}
			else 
			{
				*(merge_array->x_array + count) = *(peaks_array->x_array + peak_pos);
				*(merge_array->y_array + count) = *(peaks_array->y_array + peak_pos);
				peak_pos++;
				valley_pos++;
				count++;
				i+=2;
			}
		}
		else if(peak_pos >= peaks_array->num)
		{
			*(merge_array->x_array + count) = *(valleys_array->x_array + valley_pos);
			*(merge_array->y_array + count) = *(valleys_array->y_array + valley_pos);
			valley_pos++;
			count++;
			i++;
		}
		else
		{
			*(merge_array->x_array + count) = *(peaks_array->x_array + peak_pos);
			*(merge_array->y_array + count) = *(peaks_array->y_array + peak_pos);
			peak_pos++;
			count++;
			i++;
		}
	}

	merge_array->num = count;

}

static void peaks_valleys_merge_array_arrange(xy_data_array* src_merge_array,xy_data_array* dis_merge_array)
{
	unsigned short i = 0, j = 0,dalt_x = 0 , dalt_y = 0;
	unsigned int value = 0;

	dis_merge_array->num = 0;
	QPRINTF("**** liuyuanbin  arrange:\r\n");
	if(src_merge_array->num == 2)
	{
		
		dalt_x = *(src_merge_array->x_array + i + 1) - *(src_merge_array->x_array + i);
		value = *(src_merge_array->y_array + i);
		dalt_y = value > *(src_merge_array->y_array + i + 1) ? value - *(src_merge_array->y_array + i + 1) : *(src_merge_array->y_array + i + 1) - value;
		QPRINTF("1[x=%d,y=%d],",dalt_x,dalt_y);
		if(dalt_x >= X_AXIS_RUN_MIN_THRESHOLD && dalt_y >= Y_AXIS_DALT_DATA)
		{
			*(dis_merge_array->x_array + j) = dalt_x;
			*(dis_merge_array->y_array + j) = dalt_y;
			dis_merge_array->num = 1;
		}
		
	}
	else if(src_merge_array->num > 2)
	{
		while(i < src_merge_array->num - 2)
		{
			dalt_x = *(src_merge_array->x_array + i + 2) - *(src_merge_array->x_array + i);
			value = (*(src_merge_array->y_array + i + 2) + *(src_merge_array->y_array + i)) / 2;
			dalt_y = value > *(src_merge_array->y_array + i + 1) ? value - *(src_merge_array->y_array + i + 1) : *(src_merge_array->y_array + i + 1) - value;
			QPRINTF("2[x=%d,y=%d],",dalt_x,dalt_y);
			if(dalt_x >= X_AXIS_RUN_MIN_THRESHOLD && dalt_y >= Y_AXIS_DALT_DATA)
			{
				*(dis_merge_array->x_array + j) = dalt_x;
				*(dis_merge_array->y_array + j) = dalt_y;

				j++;
				i = i + 2;
			}
			else
			{
				i++;
			}

			if(i == src_merge_array->num - 2)
			{
				dalt_x = *(src_merge_array->x_array + i + 1) - *(src_merge_array->x_array + i);
				value =  *(src_merge_array->y_array + i);
				dalt_y = value > *(src_merge_array->y_array + i + 1) ? value - *(src_merge_array->y_array + i + 1) : *(src_merge_array->y_array + i + 1) - value;
				QPRINTF("3[x=%d,y=%d],",dalt_x,dalt_y);
				if(dalt_x >= X_AXIS_RUN_MIN_THRESHOLD && dalt_y >= Y_AXIS_DALT_DATA)
				{
					*(dis_merge_array->x_array + j) = dalt_x;
					*(dis_merge_array->y_array + j) = dalt_y;

					j++;
				}
			}

			dis_merge_array->num = j;

			
		}
	}
	QPRINTF("\r\n**************\r\n\r\n");

}


static unsigned char peaks_valleys_final_match(xy_data_array* merge_array, unsigned char* pSportMode,unsigned char hand)
{
	unsigned short temp_dalt_x = 0, temp_dalt_y = 0;
	signed short i = 0, walk_count = 0, run_count = 0;
	unsigned char steps = 0;

	unsigned short x_walk_value = 0;

	if(avg_acc_value >= 9000 && avg_acc_value <= 13000)
	{
		x_walk_value = 80;
	}
	else if(avg_acc_value > 13000 && avg_acc_value <= 17000)
	{
		x_walk_value = 60;
	}
	else if(avg_acc_value > 17000 && avg_acc_value <= 25000)
	{
		x_walk_value = 40;
	}
	else
	{
		if(avg_acc_value > 25000)
			x_walk_value = 30;
		else
			x_walk_value = 80;
	}
	
	for(i = 0; i < merge_array->num; i++)
	{
		temp_dalt_x = *(merge_array->x_array + i);
		temp_dalt_y = *(merge_array->y_array + i);
		if(temp_dalt_x < x_walk_value)
		{
			if(temp_dalt_y < Y_AXIS_DALT_WALK_MIN_THRESHOLD)
			{
				walk_count++;
			}
			else if(temp_dalt_y < Y_AXIS_DALT_RUN_MIN_THRESHOLD)
			{
				run_count++;
			}
		}
	}

	MY_QPRINTF("walk_count=%d,run_count=%d,\r\n",walk_count,run_count);

	if(run_count >= walk_count)
	{
		*pSportMode = 1;
		steps = run_count+walk_count;
		//steps = steps > 4 ? 4 : steps;
		steps = steps > 2 ? 2 : steps;
	}
	else
	{
		*pSportMode = 0;
		steps = walk_count;
		steps = steps > 1 ? 1 : steps;
	}

	return steps;
}

unsigned char deal_raw_data(unsigned short *y_array, unsigned short length, unsigned char* sport_mode,unsigned char hand)
{
	unsigned short i =0;
	unsigned char steps = 0;
	unsigned short *fir_array = NULL,avg_acc_value = 0;
	unsigned int sum = 0;
	signed short *derivative_array = NULL;
	xy_data_array peaks_array = {0};
	xy_data_array valleys_array = {0};
	xy_data_array merge_array = {0};
	xy_data_array merge_array_arrange = {0};
	
	unsigned short buf_size_256_1[BLE_ARTH_BUF_SIZE]={0};
	signed short buf_size_256_2[BLE_ARTH_BUF_SIZE]={0};
	signed short buf_size_256_3[BLE_ARTH_BUF_SIZE]={0};
	signed short buf_size_256_4[BLE_ARTH_BUF_SIZE]={0};

	fir_array = (unsigned short*)buf_size_256_1;
	memset(fir_array,0,length*2);
	fir_process(y_array,fir_array,length,FIR_SMOOTH_INTERVAL);// 滤波

	sum  = 0;
	for(i=0;i<length;i++)
		sum += fir_array[i];
	avg_acc_value = sum / length;
	MY_QPRINTF("avg_acc_value=%d,\r\n",avg_acc_value);
#if 0
		MY_QPRINTF("01");
		for(i=0;i<length;i++)
			MY_QPRINTF(",%d",fir_array[i]);
		MY_QPRINTF("\r\n");
#endif	

	derivative_array = (signed short*)buf_size_256_2;
	memset(derivative_array,0,length*2);
	derivative(length,fir_array,derivative_array);// 求导	

	peaks_array.x_array = (unsigned short*)buf_size_256_3;
	peaks_array.y_array = (unsigned short*)(buf_size_256_3+1*(BLE_ARTH_BUF_SIZE/2));
	valleys_array.x_array = (unsigned short*)(buf_size_256_4);
	valleys_array.y_array = (unsigned short*)(buf_size_256_4+1*(BLE_ARTH_BUF_SIZE/2));
	memset(peaks_array.x_array,0,length);
	memset(peaks_array.y_array,0,length);
	memset(valleys_array.x_array,0,length);
	memset(valleys_array.y_array,0,length);

	peaks_array.num = 0;
	find_peaks(fir_array,derivative_array,length,0,&peaks_array);// 找波峰
	if(peaks_array.num == 0)
	{
		QPRINTF("peaks_array.num=%d,\r\n",peaks_array.num);
		goto MEM_FREE;
	}
	#if 0
	QPRINTF("find peaks:\r\n");
	for(i=0;i<peaks_array.num;i++)
		QPRINTF("[%d,%d] ",*(peaks_array.x_array+i),*(peaks_array.y_array+i));
	QPRINTF("\r\n");
	#endif
	valleys_array.num = 0;
	find_valleys(fir_array,derivative_array,length,0,&valleys_array);
	if(valleys_array.num == 0)
	{
		QPRINTF("valleys_array.num=%d,\r\n",valleys_array.num);
		goto MEM_FREE;
	}
#if 0
	QPRINTF("find valleys:\r\n");
	for(i=0;i<valleys_array.num;i++)
		QPRINTF("[%d,%d] ",*(valleys_array.x_array+i),*(valleys_array.y_array+i));
	QPRINTF("\r\n");
#endif	
	merge_array.x_array= (unsigned short*)buf_size_256_1;
	merge_array.y_array= (unsigned short*)(buf_size_256_1+(BLE_ARTH_BUF_SIZE/2));
	peaks_valleys_merge(&peaks_array,&valleys_array,&merge_array);//合并
	if(merge_array.num <= 1 || merge_array.num >= 12)
	{
		QPRINTF("merge_array.num=%d,\r\n",merge_array.num);
		goto MEM_FREE;
	}
#if 0
	QPRINTF("merge_array:\r\n");
	for(i=0;i<merge_array.num;i++)
		QPRINTF("[%d,%d] ",*(merge_array.x_array+i),*(merge_array.y_array+i));
	QPRINTF("\r\n");
#endif	
	merge_array_arrange.x_array= (unsigned short*)buf_size_256_2;
	merge_array_arrange.y_array= (unsigned short*)(buf_size_256_2+(BLE_ARTH_BUF_SIZE/2));
	memset(merge_array_arrange.x_array,0,length);
	memset(merge_array_arrange.y_array,0,length);
	peaks_valleys_merge_array_arrange(&merge_array,&merge_array_arrange);	
	if(merge_array_arrange.num < 1)
		goto MEM_FREE;
#if 0
	QPRINTF("merge_array_arrange:\r\n");
	for(i=0;i<merge_array_arrange.num;i++)
		QPRINTF("[%d,%d] ",*(merge_array_arrange.x_array+i),*(merge_array_arrange.y_array+i));
	QPRINTF("\r\n\r\n");
#endif	
	steps = peaks_valleys_final_match(&merge_array_arrange, sport_mode,avg_acc_value,hand);

MEM_FREE:
	return steps;
}
#elif USE_ARITHMETIC_FROM == WUFAN_ARITHMETIC

static unsigned short g_yAxisDaltMinThreshold = Y_AXIS_DALT_DAY_MIN_THRESHOLD;

void Arithmetic_Set_YAxis_Min_Threshold(unsigned short data) 
{
	g_yAxisDaltMinThreshold = data;
}

static void FIRProcess(unsigned short *InData, unsigned short length, unsigned short *OutData)
{
	unsigned int sum = 0; 
	unsigned short i;
	unsigned short  j = 0;
	if(length > FIR_SMOOTH_INTERVAL)
	{
		for(i = 0;i < length - FIR_SMOOTH_INTERVAL;i++)
		{	
			sum = 0;
			for(j = 0;j < FIR_SMOOTH_INTERVAL;j++)
			{
				sum += InData[i+j];  
			} 
			OutData[i] = sum/FIR_SMOOTH_INTERVAL;
		}

		for(i = length - FIR_SMOOTH_INTERVAL; i < length; i++)
		{	
			sum = 0;
			for(j = 0;j < FIR_SMOOTH_INTERVAL;j++)
			{
				sum += InData[i-j];  
			}
			OutData[i] = sum/FIR_SMOOTH_INTERVAL;
		}
	}
	else
	{
		for(i = 0; i < length; i++)
			OutData[i] = InData[i];
	}
	return;
}

/* derivative function input param: 
 * src : input vector 
 * length : input vector length
 * dist: output result vector  */
static void derivativeFunc(unsigned short length, unsigned short *src, signed short *dist)
{
	unsigned short i = 0;
	if(length > 2)
	{
		#if 1
		*dist = *(src+1) - *src;
		*(dist + length - 1) = *(src + length - 1) - *(src + length - 2);

		for(i = 1; i < length - 2; i++)
			*(dist + i) = (*(src + i + 1) - *(src + i - 1))/2;
		#else
		for(i = 0; i < length - 2; i++)
			*(dist + i) = (*(src + i + 1) - *(src + i))/2;
		#endif
	}
	else
	{
		for(i = 0; i < length; i++)
			*(dist + i) = *(src + i);
	}
	return;
}

/* smooth function input param: 
 * src : input vector 
 * length : input vector length
 * dist: output result vector 
 * sw : width 
 * ends: end points, "ends" controls how the "ends" of the signal */
static void smoothfunc(unsigned short length, unsigned short ends,signed short* src, signed short* dist)
{
	unsigned short sw = SMOOTH_WIDTH;
	unsigned short i = 0, j = 0, halfSW = 0, startPoint = 0;
	signed int sumVal = 0;
	if(length <= sw)
		sw = length/2;
	halfSW = sw/2;

	if(length <= 2)
	{
		for(i = 0; i < length; i++)
			*(dist + i) = *(src + i);
		return;
	}
	
	for(i = 0; i < sw; i++)
		sumVal += *(src+i);
		
	for(i = 0; i < length - sw -1; i++)
	{
		*(dist + i +halfSW - 2) = (signed short)sumVal;
		sumVal -= *(src + i);
		sumVal += *(src + i + sw - 1);
	}
	
	sumVal = 0;
	for(j = length-sw -1; j < length; j++)
		sumVal += *(src + j);
		
	*(dist + i +halfSW - 1) = (signed short)sumVal;
	for(j = 0; j < length; j++)
	{
		*(dist + j) = *(dist + j)/sw;
	}
	
	if(ends == 1)
	{
		startPoint = sw/2;
		*dist = (*src + *(src+1))/2;
		for(i = 1; i < startPoint; i++)
		{
			sumVal = 0;
			for(j = 0; j < 2*i - 1; j++)
				sumVal += *(src+j);
			*(dist + i) = (signed short)(sumVal/j);
			
			sumVal = 0;
			for(j = length - 2*i + 1; j < length; j++)
				sumVal += *(src+j);
			*(dist + length - i - 1) = (signed short)(sumVal/(2*i - 1));
		}
		*(dist + length - 1) = (*(src + length - 1) + *(src + length - 2))/2;
	}

	return;
}

static unsigned short FindPeaks(unsigned short *y, unsigned short length,unsigned short AmpThreshold, signed short* smoothYArray, XYDataArray* pPeaksArray)
{
	unsigned short j = 0, peakCount = 0, startPos = 0;
	unsigned short smoothwidth = SMOOTH_WIDTH;
	if(length <= smoothwidth)
		smoothwidth = length/2;

	if(length <= 2)
	{
		return length;
	}
	
    if(smoothwidth > 1)
		startPos = 2*(smoothwidth/2) - 1;
	else
		startPos = 1;

    for (j = startPos-1 ; j < length - smoothwidth - 1 ;j++)
    {
        if(((*(smoothYArray+j) > 0)&&(*(smoothYArray+j+1) <= 0)) ||
			((*(smoothYArray+j) == 0) && (*(smoothYArray+j+1) < 0)))//Detects zero-crossing
        {
            if(*(y + j) > AmpThreshold)   //  if height of peak is larger than AmpThreshold
            {
				*(pPeaksArray->xArray + peakCount) = j;
				*(pPeaksArray->yArray + peakCount) = *(y + j);
				peakCount++;
            } 
        }
    }
	pPeaksArray->num = peakCount;
	return peakCount;
}

static unsigned short FindValleys(unsigned short *y,unsigned short length,unsigned short AmpThreshold, signed short* smoothYArray, XYDataArray* pValleysArray)
{
	unsigned short j = 0, valleyCount = 0, startPos = 0;
	unsigned short smoothwidth = SMOOTH_WIDTH;
	if(length <= smoothwidth)
		smoothwidth = length/2;

	if(length <= 2)
	{
		return length;
	}
	
    if(smoothwidth > 1)
		startPos = 2*(smoothwidth/2) - 1;
	else
		startPos = 1;

    for (j = startPos-1 ; j < length - smoothwidth - 1 ;j++)
    {
        if(((*(smoothYArray+j) <= 0)&&(*(smoothYArray+j+1) > 0)) ||
			((*(smoothYArray+j) < 0) && (*(smoothYArray+j+1) == 0)))//Detects zero-crossing
        {       
            if(*(y + j) > AmpThreshold)   //  if height of peak is larger than AmpThreshold
            {                 
				*(pValleysArray->xArray + valleyCount) = j;
				*(pValleysArray->yArray + valleyCount) = *(y + j);
				valleyCount++;
                    
            }
        }
    }
	pValleysArray->num = valleyCount;
	return valleyCount;
}

static void peaks_valleys_Merge(XYDataArray* pPeaksArray, XYDataArray* pValleysArray, XYDataArray* pMergeArray)
{
	unsigned short i = 0, count = 0, peakPos = 0, valleyPos = 0;
	
	memset(pMergeArray->xArray,0,BLE_ARTH_BUF_SIZE);
	memset(pMergeArray->yArray,0,BLE_ARTH_BUF_SIZE);
	pMergeArray->num = 0;
	
	while(i < (pPeaksArray->num + pValleysArray->num))
	{
		if((peakPos < pPeaksArray->num) && (valleyPos < pValleysArray->num))
		{
			if(*(pPeaksArray->xArray + peakPos) < *(pValleysArray->xArray + valleyPos))
			{
				*(pMergeArray->xArray + count) = *(pPeaksArray->xArray + peakPos);
				*(pMergeArray->yArray + count) = *(pPeaksArray->yArray + peakPos);
				peakPos++;
				count++;
				i++;
			}
			else if(*(pPeaksArray->xArray + peakPos) > *(pValleysArray->xArray + valleyPos))
			{
				*(pMergeArray->xArray + count) = *(pValleysArray->xArray + valleyPos);
				*(pMergeArray->yArray + count) = *(pValleysArray->yArray + valleyPos);
				valleyPos++;
				count++;
				i++;
			}
			else 
			{
				*(pMergeArray->xArray + count) = *(pPeaksArray->xArray + peakPos);
				*(pMergeArray->yArray + count) = *(pPeaksArray->yArray + peakPos);
				peakPos++;
				valleyPos++;
				count++;
				i+=2;
			}
		}
		else if(peakPos >= pPeaksArray->num)
		{
			*(pMergeArray->xArray + count) = *(pValleysArray->xArray + valleyPos);
			*(pMergeArray->yArray + count) = *(pValleysArray->yArray + valleyPos);
			valleyPos++;
			count++;
			i++;
		}
		else
		{
			*(pMergeArray->xArray + count) = *(pPeaksArray->xArray + peakPos);
			*(pMergeArray->yArray + count) = *(pPeaksArray->yArray + peakPos);
			peakPos++;
			count++;
			i++;
		}
	}

	pMergeArray->num = count;
#if ARI_MODULE_LOG_EN
	ARITH_INFO("\r\n %d totalPeaksAndValleys array: \r\n",count);
	for(i = 0; i < count; i++)
	{
		ARITH_INFO("[%d] %d, ",*(pMergeArray->xArray + i), *(pMergeArray->yArray + i));
	}
	ARITH_INFO("\r\n");
#endif	
}

static void peaksValleys_mergeArray_arrange(XYDataArray* pMergeArray)
{
	if(pMergeArray->num < 2)
		return;
	
	unsigned short arrangeXArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short arrangeYArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short i = 0, j = 0,daltX = 0, daltY = 0, minYVal = 0, count = 0, beginPos = 0, find = 0;
	
	for(i = 0; i < pMergeArray->num - 1; i++)
	{
		daltX = *(pMergeArray->xArray + i + 1) - *(pMergeArray->xArray + i);
		daltY = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i + 1) - *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i)  - *(pMergeArray->yArray + i + 1) ;
		minYVal = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i + 1);
		if(daltX <= 5 && daltY <= minYVal/8)
		{
			if(find == 0)
			{
				find = 1;
				beginPos = i;
			}
		}
		else
		{
			if(find == 1)
			{
				unsigned int sumX = 0, sumY = 0;
				unsigned short avgX = 0, avgY = 0;
				find = 0;
				for(j = beginPos; j <= i; j++)
				{
					sumX += *(pMergeArray->xArray + j);
					sumY += *(pMergeArray->yArray + j);
				}
			
				avgX = sumX/(i - beginPos+1);
				avgY = sumY/(i - beginPos+1);

				*(arrangeXArray + count) = avgX;
				*(arrangeYArray + count) = avgY;
				count++;
			}
			else
			{
				*(arrangeXArray + count) = *(pMergeArray->xArray + i);
				*(arrangeYArray + count) = *(pMergeArray->yArray + i);
				count++;
			}
		}
	}

	if(find == 1)
	{
		unsigned int sumX = 0, sumY = 0;
		unsigned short avgX = 0, avgY = 0;
		find = 0;
		for(j = beginPos; j <= i; j++)
		{
			sumX += *(pMergeArray->xArray + j);
			sumY += *(pMergeArray->yArray + j);
		}
	
		avgX = sumX/(i - beginPos+1);
		avgY = sumY/(i - beginPos+1);

		*(arrangeXArray + count) = avgX;
		*(arrangeYArray + count) = avgY;
		count++;
	}
	else
	{
		if(i == pMergeArray->num - 1)
		{
			*(arrangeXArray + count) = *(pMergeArray->xArray + i);
			*(arrangeYArray + count) = *(pMergeArray->yArray + i);
			count++;
		}
	}
	memset(pMergeArray->xArray, 0 ,pMergeArray->num*2);
	memset(pMergeArray->yArray, 0 ,pMergeArray->num*2);
	pMergeArray->num = count;
	memcpy(pMergeArray->xArray, arrangeXArray ,count*2);
	memcpy(pMergeArray->yArray, arrangeYArray ,count*2);

#if ARI_MODULE_LOG_EN
	ARITH_INFO("\r\n arrangeMergeArray %d items: \r\n",count);
	for(i = 0; i < count; i++)
	{
		ARITH_INFO("[%d] %d, ",*(pMergeArray->xArray + i), *(pMergeArray->yArray + i));
	}
	ARITH_INFO("\r\n");
#endif

}

static unsigned char steps_matchArray_verfy(unsigned short* xArray, unsigned short* yArray, unsigned short arrayNum, unsigned char sportMode)
{
	unsigned short i = 0, xDaltMin = 0, invalidTotalCount = 0, disconnectCount = 0, onceMaxNum = 0, validTotalCount = 0, steps = 0, find = 0, daltX = 0, beginPos = 0;
	if(sportMode) // run mode
		xDaltMin = X_AXIS_RUN_MIN_THRESHOLD;
	else // walk mode
		xDaltMin = X_AXIS_WALK_MIN_THRESHOLD;

	while(i < arrayNum - 1)
	{
		daltX = *(xArray + i + 1) - *(xArray + i);
		if(daltX >= xDaltMin)
		{
			if(find == 0)
			{
				find = 1;
				beginPos = i;	
			}
			validTotalCount++;
		}
		else
		{
			if(find == 1)
			{
				find = 0;
				disconnectCount++;
				if(i - beginPos > onceMaxNum)
					onceMaxNum = i - beginPos;
			}
			invalidTotalCount++;
		}
		i++;
	}

	if(find == 1)
	{
		if(i - beginPos > onceMaxNum)
			onceMaxNum = i - beginPos;
	}

	ARITH_INFO("i = %d, arrayNum = %d, sportMode = %d,validTotalCount = %d,onceMaxNum = %d,invalidTotalCount = %d, disconnectCount = %d\r\n",
		i,arrayNum,sportMode,validTotalCount,onceMaxNum,invalidTotalCount,disconnectCount);

 	if((validTotalCount > invalidTotalCount) && (validTotalCount > disconnectCount))
 	{
 	//	if(onceMaxNum > validTotalCount/2)
			steps = onceMaxNum;	
	//	else
	//		steps = validTotalCount;
	}

	if(steps > 5)
		return 0;
	
	if(sportMode)
		steps = steps > 4 ? 4 : steps;
	else
		steps = steps > 2 ? 2 : steps;
	return steps;
}

static unsigned char peaks_valleys_final_match(XYDataArray* pMergeArray, unsigned char* pSportMode)
{
	unsigned short tempDaltX = 0, tempDaltY = 0, tempMinY = 0;
	signed short i = 0, walkCount = 0, runCount = 0, invalidXCount = 0, invalidXYCount = 0, steps = 0;
	unsigned short pWalkXArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short pWalkYArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short pRunXArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short pRunYArray[BLE_ARTH_BUF_SIZE/2];
	
	for(i = 0; i < pMergeArray->num - 1; i++)
	{
		tempDaltX = *(pMergeArray->xArray + i + 1) - *(pMergeArray->xArray + i);
		tempDaltY = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i + 1) - *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i) - *(pMergeArray->yArray + i + 1);
		tempMinY = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i + 1);
		ARITH_INFO("i = %d, tempDaltX = %d, tempDaltY = %d, tempMinY = %d\r\n",i,tempDaltX,tempDaltY,tempMinY);

		if((tempDaltX >= X_AXIS_RUN_MIN_THRESHOLD) && (tempDaltY > g_yAxisDaltMinThreshold && tempDaltY > tempMinY/5 && tempDaltY < tempMinY*5))
		{
			ARITH_INFO("Run: runCount = %d, runx = %d,runy = %d, mergeX[%d] = %d, mergeY[%d] = %d, mergeX[%d] = %d, mergeY[%d] = %d\r\n",
				runCount, runCount > 0 ? *(pRunXArray + runCount - 1): 0, runCount > 0 ? *(pRunXArray + runCount - 1): 0,
				i,*(pMergeArray->xArray + i),i,*(pMergeArray->yArray + i),i+1,*(pMergeArray->xArray + i + 1),i+1,*(pMergeArray->yArray + i + 1));
			if(runCount == 0 || *(pRunXArray + runCount - 1) != *(pMergeArray->xArray + i))
			{
				*(pRunXArray+runCount) = *(pMergeArray->xArray + i);
				*(pRunYArray+runCount) = *(pMergeArray->yArray + i);
				runCount++;
			}

			if(runCount == 0 || *(pRunXArray + runCount - 1) != *(pMergeArray->xArray + i + 1))
			{
				*(pRunXArray+runCount) = *(pMergeArray->xArray + i + 1);
				*(pRunYArray+runCount) = *(pMergeArray->yArray + i + 1);
				runCount++;
			}

			if((tempDaltX >= X_AXIS_WALK_MIN_THRESHOLD) && (tempDaltY > g_yAxisDaltMinThreshold && tempDaltY > tempMinY/3 && tempDaltY < tempMinY*5))
		//	if(tempDaltX >= X_AXIS_WALK_MIN_THRESHOLD)
			{
				ARITH_INFO("Walk: walkCount = %d, walkx = %d, walky = %d, mergeX[%d] = %d, mergeY[%d] = %d, mergeX[%d] = %d, mergeY[%d] = %d\r\n",
					walkCount, walkCount > 0 ? *(pWalkXArray + walkCount - 1): 0, walkCount > 0 ? *(pWalkYArray + walkCount - 1): 0,
					i,*(pMergeArray->xArray + i),i,*(pMergeArray->yArray + i),i+1,*(pMergeArray->xArray + i + 1),i+1,*(pMergeArray->yArray + i + 1));
				if(walkCount == 0 || *(pWalkXArray + walkCount - 1) != *(pMergeArray->xArray + i))
				{
					*(pWalkXArray+walkCount) = *(pMergeArray->xArray + i);
					*(pWalkYArray+walkCount) = *(pMergeArray->yArray + i);
					walkCount++;
				}

				if(walkCount == 0 || *(pWalkXArray + walkCount - 1) != *(pMergeArray->xArray + i + 1))
				{
					*(pWalkXArray+walkCount) = *(pMergeArray->xArray + i + 1);
					*(pWalkYArray+walkCount) = *(pMergeArray->yArray + i + 1);
					walkCount++;
				}
			}
		}
		else if((tempDaltX < X_AXIS_RUN_MIN_THRESHOLD) && (tempDaltY > g_yAxisDaltMinThreshold && tempDaltY > tempMinY/5 && tempDaltY < tempMinY*5))
		{
			invalidXCount++;
		}
		else if(tempDaltX >= X_AXIS_RUN_MIN_THRESHOLD)
		{
			invalidXYCount++;
		}
	}

#if ARI_MODULE_LOG_EN
	ARITH_INFO("\r\n invalidXCount = %d\r\nWalk Temp Array %d items: \r\n",invalidXCount, walkCount);
	for(i = 0; i < walkCount; i++)
	{
		ARITH_INFO("[%d :% d], ",*(pWalkXArray+i),*(pWalkYArray+i));
	}
	ARITH_INFO("\r\nRun Temp Array %d items: \r\n", walkCount);
	for(i = 0; i < runCount; i++)
	{
		ARITH_INFO("[%d :% d], ",*(pRunXArray+i),*(pRunYArray+i));
	}
	ARITH_INFO("\r\n");
#endif


	if((walkCount >= 2 && (runCount - 1) >= 3*(walkCount - 1)) ||
		(walkCount < 2))
	{
		*pSportMode = 1;

		if((invalidXCount <= runCount*3/2) && (invalidXCount + invalidXYCount <= runCount*2))
			steps = steps_matchArray_verfy(pRunXArray,pRunYArray,runCount,1);
	}
	else
	{
		*pSportMode = 0;

		steps = steps_matchArray_verfy(pWalkXArray,pWalkYArray,walkCount,0);
	}

	return steps;
}

unsigned char deal_raw_data(unsigned short *yArray, unsigned short length, unsigned char* pSportMode,unsigned char hand)
{
	unsigned char steps = 0;
	unsigned short *firYArray = NULL;
	signed short *derivativeYArray = NULL;
	signed short *smoothYArray = NULL;
	XYDataArray peaksArray = {0};
	XYDataArray valleysArray = {0};
	XYDataArray mergeArray = {0};
	
	unsigned short buf_size_256_1[BLE_ARTH_BUF_SIZE]={0};
	signed short buf_size_256_2[BLE_ARTH_BUF_SIZE]={0};
	signed short buf_size_256_3[BLE_ARTH_BUF_SIZE]={0};

	firYArray = (unsigned short*)buf_size_256_1;
	memset(firYArray,0,length*2);
	FIRProcess(yArray,length,firYArray);// 滤波

	derivativeYArray = (signed short*)buf_size_256_2;
	memset(derivativeYArray,0,length*2);
	derivativeFunc(length,yArray,derivativeYArray);// 求导

	smoothYArray = (signed short*)buf_size_256_3;
	memset(smoothYArray,0,length*2);
	smoothfunc(length,0,derivativeYArray,smoothYArray);//平滑


	peaksArray.xArray = (unsigned short*)buf_size_256_1;
	peaksArray.yArray = (unsigned short*)(buf_size_256_1+1*(BLE_ARTH_BUF_SIZE/2));
	valleysArray.xArray = (unsigned short*)(buf_size_256_2);
	valleysArray.yArray = (unsigned short*)(buf_size_256_2+1*(BLE_ARTH_BUF_SIZE/2));
	memset(peaksArray.xArray,0,length);
	memset(peaksArray.yArray,0,length);
	memset(valleysArray.xArray,0,length);
	memset(valleysArray.yArray,0,length);


	peaksArray.num = 0;
	FindPeaks(yArray,length,0,smoothYArray,&peaksArray);// 找波峰
	if(peaksArray.num == 0)
	{
		goto MEM_FREE;
	}

	valleysArray.num = 0;
	FindValleys(yArray,length,0,smoothYArray,&valleysArray);
	if(valleysArray.num == 0)
	{
		goto MEM_FREE;
	}

	mergeArray.xArray = (unsigned short*)buf_size_256_3;
	mergeArray.yArray = (unsigned short*)(buf_size_256_3+(BLE_ARTH_BUF_SIZE/2));
	peaks_valleys_Merge(&peaksArray,&valleysArray,&mergeArray);//合并
	
	peaksValleys_mergeArray_arrange(&mergeArray);

	if(mergeArray.num <= 1)
		goto MEM_FREE;
	
	steps = peaks_valleys_final_match(&mergeArray, pSportMode);

MEM_FREE:
	return steps;
}


#elif USE_ARITHMETIC_FROM == LYB_ARITHMETIC
static unsigned short g_yAxisDaltMinThreshold = Y_AXIS_DALT_DAY_MIN_THRESHOLD;
unsigned short x_axis_run_min_threshold 	= X_AXIS_RUN_MIN_THRESHOLD;
unsigned short x_axis_walk_min_threshold 	= X_AXIS_WALK_MIN_THRESHOLD;
unsigned short yAverage = 0,yMax=0,yMin=0;

void Arithmetic_Set_YAxis_Min_Threshold(unsigned short data) 
{
	g_yAxisDaltMinThreshold = data;
}

static void FIRProcess(unsigned short *InData, unsigned short length, unsigned short *OutData)
{
	unsigned int sum = 0; 
	unsigned short i;
	unsigned short  j = 0;
	if(length > FIR_SMOOTH_INTERVAL)
	{
		for(i = 0;i < length - FIR_SMOOTH_INTERVAL;i++)
		{	
			sum = 0;
			for(j = 0;j < FIR_SMOOTH_INTERVAL;j++)
			{
				sum += InData[i+j];  
			} 
			OutData[i] = sum/FIR_SMOOTH_INTERVAL;
		}

		for(i = length - FIR_SMOOTH_INTERVAL; i < length; i++)
		{	
			sum = 0;
			for(j = 0;j < FIR_SMOOTH_INTERVAL;j++)
			{
				sum += InData[i-j];  
			}
			OutData[i] = sum/FIR_SMOOTH_INTERVAL;
		}
	}
	else
	{
		for(i = 0; i < length; i++)
			OutData[i] = InData[i];
	}
	return;
}

/* derivative function input param: 
 * src : input vector 
 * length : input vector length
 * dist: output result vector  */
static void derivativeFunc(unsigned short length, unsigned short *src, signed short *dist)
{
	unsigned short i = 0;
	if(length > 2)
	{
		#if 1
		*dist = *(src+1) - *src;
		*(dist + length - 1) = *(src + length - 1) - *(src + length - 2);

		for(i = 1; i < length - 2; i++)
			*(dist + i) = (*(src + i + 1) - *(src + i - 1))/2;
		#else
		for(i = 0; i < length - 2; i++)
			*(dist + i) = (*(src + i + 1) - *(src + i))/2;
		#endif
	}
	else
	{
		for(i = 0; i < length; i++)
			*(dist + i) = *(src + i);
	}
	return;
}

/* smooth function input param: 
 * src : input vector 
 * length : input vector length
 * dist: output result vector 
 * sw : width 
 * ends: end points, "ends" controls how the "ends" of the signal */
static void smoothfunc(unsigned short length, unsigned short ends,signed short* src, signed short* dist)
{
	unsigned short sw = SMOOTH_WIDTH;
	unsigned short i = 0, j = 0, halfSW = 0, startPoint = 0;
	signed int sumVal = 0;
	if(length <= sw)
		sw = length/2;
	halfSW = sw/2;

	if(length <= 2)
	{
		for(i = 0; i < length; i++)
			*(dist + i) = *(src + i);
		return;
	}
	
	for(i = 0; i < sw; i++)
		sumVal += *(src+i);
		
	for(i = 0; i < length - sw -1; i++)
	{
		*(dist + i +halfSW - 2) = (signed short)sumVal;
		sumVal -= *(src + i);
		sumVal += *(src + i + sw - 1);
	}
	
	sumVal = 0;
	for(j = length-sw -1; j < length; j++)
		sumVal += *(src + j);
		
	*(dist + i +halfSW - 1) = (signed short)sumVal;
	for(j = 0; j < length; j++)
	{
		*(dist + j) = *(dist + j)/sw;
	}
	
	if(ends == 1)
	{
		startPoint = sw/2;
		*dist = (*src + *(src+1))/2;
		for(i = 1; i < startPoint; i++)
		{
			sumVal = 0;
			for(j = 0; j < 2*i - 1; j++)
				sumVal += *(src+j);
			*(dist + i) = (signed short)(sumVal/j);
			
			sumVal = 0;
			for(j = length - 2*i + 1; j < length; j++)
				sumVal += *(src+j);
			*(dist + length - i - 1) = (signed short)(sumVal/(2*i - 1));
		}
		*(dist + length - 1) = (*(src + length - 1) + *(src + length - 2))/2;
	}

	return;
}

static unsigned short FindPeaks(unsigned short *y, unsigned short length,unsigned short AmpThreshold, signed short* smoothYArray, XYDataArray* pPeaksArray)
{
	unsigned short j = 0, peakCount = 0, startPos = 0;
	unsigned short smoothwidth = SMOOTH_WIDTH;
	if(length <= smoothwidth)
		smoothwidth = length/2;

	if(length <= 2)
	{
		return length;
	}
	
    if(smoothwidth > 1)
		startPos = 2*(smoothwidth/2) - 1;
	else
		startPos = 1;

    for (j = startPos-1 ; j < length - smoothwidth - 1 ;j++)
    {
        if(((*(smoothYArray+j) > 0)&&(*(smoothYArray+j+1) <= 0)) ||
			((*(smoothYArray+j) == 0) && (*(smoothYArray+j+1) < 0)))//Detects zero-crossing
        {
            if(*(y + j) > AmpThreshold)   //  if height of peak is larger than AmpThreshold
            {
				*(pPeaksArray->xArray + peakCount) = j;
				*(pPeaksArray->yArray + peakCount) = *(y + j);
				peakCount++;
            } 
        }
    }
	pPeaksArray->num = peakCount;
	return peakCount;
}

static unsigned short FindValleys(unsigned short *y,unsigned short length,unsigned short AmpThreshold, signed short* smoothYArray, XYDataArray* pValleysArray)
{
	unsigned short j = 0, valleyCount = 0, startPos = 0;
	unsigned short smoothwidth = SMOOTH_WIDTH;
	if(length <= smoothwidth)
		smoothwidth = length/2;

	if(length <= 2)
	{
		return length;
	}
	
    if(smoothwidth > 1)
		startPos = 2*(smoothwidth/2) - 1;
	else
		startPos = 1;

    for (j = startPos-1 ; j < length - smoothwidth - 1 ;j++)
    {
        if(((*(smoothYArray+j) <= 0)&&(*(smoothYArray+j+1) > 0)) ||
			((*(smoothYArray+j) < 0) && (*(smoothYArray+j+1) == 0)))//Detects zero-crossing
        {       
            if(*(y + j) > AmpThreshold)   //  if height of peak is larger than AmpThreshold
            {                 
				*(pValleysArray->xArray + valleyCount) = j;
				*(pValleysArray->yArray + valleyCount) = *(y + j);
				valleyCount++;
                    
            }
        }
    }
	pValleysArray->num = valleyCount;
	return valleyCount;
}

static void peaks_valleys_Merge(XYDataArray* pPeaksArray, XYDataArray* pValleysArray, XYDataArray* pMergeArray)
{
	unsigned short i = 0, count = 0, peakPos = 0, valleyPos = 0;
	
	memset(pMergeArray->xArray,0,BLE_ARTH_BUF_SIZE);
	memset(pMergeArray->yArray,0,BLE_ARTH_BUF_SIZE);
	pMergeArray->num = 0;
	
	while(i < (pPeaksArray->num + pValleysArray->num))
	{
		if((peakPos < pPeaksArray->num) && (valleyPos < pValleysArray->num))
		{
			if(*(pPeaksArray->xArray + peakPos) < *(pValleysArray->xArray + valleyPos))
			{
				*(pMergeArray->xArray + count) = *(pPeaksArray->xArray + peakPos);
				*(pMergeArray->yArray + count) = *(pPeaksArray->yArray + peakPos);
				peakPos++;
				count++;
				i++;
			}
			else if(*(pPeaksArray->xArray + peakPos) > *(pValleysArray->xArray + valleyPos))
			{
				*(pMergeArray->xArray + count) = *(pValleysArray->xArray + valleyPos);
				*(pMergeArray->yArray + count) = *(pValleysArray->yArray + valleyPos);
				valleyPos++;
				count++;
				i++;
			}
			else 
			{
				*(pMergeArray->xArray + count) = *(pPeaksArray->xArray + peakPos);
				*(pMergeArray->yArray + count) = *(pPeaksArray->yArray + peakPos);
				peakPos++;
				valleyPos++;
				count++;
				i+=2;
			}
		}
		else if(peakPos >= pPeaksArray->num)
		{
			*(pMergeArray->xArray + count) = *(pValleysArray->xArray + valleyPos);
			*(pMergeArray->yArray + count) = *(pValleysArray->yArray + valleyPos);
			valleyPos++;
			count++;
			i++;
		}
		else
		{
			*(pMergeArray->xArray + count) = *(pPeaksArray->xArray + peakPos);
			*(pMergeArray->yArray + count) = *(pPeaksArray->yArray + peakPos);
			peakPos++;
			count++;
			i++;
		}
	}

	pMergeArray->num = count;
#if ARI_MODULE_LOG_EN
	ARITH_INFO("\r\n %d totalPeaksAndValleys array: \r\n",count);
	for(i = 0; i < count; i++)
	{
		ARITH_INFO("[%d] %d, ",*(pMergeArray->xArray + i), *(pMergeArray->yArray + i));
	}
	ARITH_INFO("\r\n");
#endif	
}

static void peaksValleys_mergeArray_arrange(XYDataArray* pMergeArray)
{
	if(pMergeArray->num < 2)
		return;
	
	unsigned short arrangeXArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short arrangeYArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short i = 0, j = 0,daltX = 0, daltY = 0, minYVal = 0, count = 0, beginPos = 0, find = 0;
	
	for(i = 0; i < pMergeArray->num - 1; i++)
	{
		daltX = *(pMergeArray->xArray + i + 1) - *(pMergeArray->xArray + i);
		daltY = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i + 1) - *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i)  - *(pMergeArray->yArray + i + 1) ;
		minYVal = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i + 1);
		if(daltX <= 5 && daltY <= minYVal/8)
		{
			if(find == 0)
			{
				find = 1;
				beginPos = i;
			}
		}
		else
		{
			if(find == 1)
			{
				unsigned int sumX = 0, sumY = 0;
				unsigned short avgX = 0, avgY = 0;
				find = 0;
				for(j = beginPos; j <= i; j++)
				{
					sumX += *(pMergeArray->xArray + j);
					sumY += *(pMergeArray->yArray + j);
				}
			
				avgX = sumX/(i - beginPos+1);
				avgY = sumY/(i - beginPos+1);

				*(arrangeXArray + count) = avgX;
				*(arrangeYArray + count) = avgY;
				count++;
			}
			else
			{
				*(arrangeXArray + count) = *(pMergeArray->xArray + i);
				*(arrangeYArray + count) = *(pMergeArray->yArray + i);
				count++;
			}
		}
	}

	if(find == 1)
	{
		unsigned int sumX = 0, sumY = 0;
		unsigned short avgX = 0, avgY = 0;
		find = 0;
		for(j = beginPos; j <= i; j++)
		{
			sumX += *(pMergeArray->xArray + j);
			sumY += *(pMergeArray->yArray + j);
		}
	
		avgX = sumX/(i - beginPos+1);
		avgY = sumY/(i - beginPos+1);

		*(arrangeXArray + count) = avgX;
		*(arrangeYArray + count) = avgY;
		count++;
	}
	else
	{
		if(i == pMergeArray->num - 1)
		{
			*(arrangeXArray + count) = *(pMergeArray->xArray + i);
			*(arrangeYArray + count) = *(pMergeArray->yArray + i);
			count++;
		}
	}
	memset(pMergeArray->xArray, 0 ,pMergeArray->num*2);
	memset(pMergeArray->yArray, 0 ,pMergeArray->num*2);
	pMergeArray->num = count;
	memcpy(pMergeArray->xArray, arrangeXArray ,count*2);
	memcpy(pMergeArray->yArray, arrangeYArray ,count*2);

#if ARI_MODULE_LOG_EN
	ARITH_INFO("\r\n arrangeMergeArray %d items: \r\n",count);
	for(i = 0; i < count; i++)
	{
		ARITH_INFO("[%d] %d, ",*(pMergeArray->xArray + i), *(pMergeArray->yArray + i));
	}
	ARITH_INFO("\r\n");
#endif

}

static unsigned char steps_matchArray_verfy(unsigned short* xArray, unsigned short* yArray, unsigned short arrayNum, unsigned char sportMode)
{
	unsigned short i = 0, xDaltMin = 0, invalidTotalCount = 0, disconnectCount = 0, onceMaxNum = 0, validTotalCount = 0, steps = 0, find = 0, daltX = 0, beginPos = 0;
	if(sportMode) // run mode
		xDaltMin = x_axis_run_min_threshold;//X_AXIS_RUN_MIN_THRESHOLD;
	else // walk mode
		xDaltMin = x_axis_walk_min_threshold;//X_AXIS_WALK_MIN_THRESHOLD;

	while(i < arrayNum - 1)
	{
		daltX = *(xArray + i + 1) - *(xArray + i);
		if(daltX >= xDaltMin)
		{
			if(find == 0)
			{
				find = 1;
				beginPos = i;	
			}
			validTotalCount++;
		}
		else
		{
			if(find == 1)
			{
				find = 0;
				disconnectCount++;
				if(i - beginPos > onceMaxNum)
					onceMaxNum = i - beginPos;
			}
			invalidTotalCount++;
		}
		i++;
	}

	if(find == 1)
	{
		if(i - beginPos > onceMaxNum)
			onceMaxNum = i - beginPos;
	}

	ARITH_INFO("i = %d, arrayNum = %d, sportMode = %d,validTotalCount = %d,onceMaxNum = %d,invalidTotalCount = %d, disconnectCount = %d\r\n",
		i,arrayNum,sportMode,validTotalCount,onceMaxNum,invalidTotalCount,disconnectCount);

 	if((validTotalCount > invalidTotalCount) && (validTotalCount > disconnectCount))
 	{
 	//	if(onceMaxNum > validTotalCount/2)
			steps = onceMaxNum;	
	//	else
	//		steps = validTotalCount;
	}

	if(steps > 5)
		return 0;
	
	if(sportMode)
		steps = steps > 4 ? 4 : steps;
	else
		steps = steps > 2 ? 2 : steps;
	return steps;
}

static unsigned char peaks_valleys_final_match(XYDataArray* pMergeArray, unsigned char* pSportMode)
{
	unsigned short tempDaltX = 0, tempDaltY = 0, tempMinY = 0;
	signed short i = 0, walkCount = 0, runCount = 0, invalidXCount = 0, invalidXYCount = 0, steps = 0;
	unsigned short pWalkXArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short pWalkYArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short pRunXArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short pRunYArray[BLE_ARTH_BUF_SIZE/2];
	
	for(i = 0; i < pMergeArray->num - 1; i++)
	{
		tempDaltX = *(pMergeArray->xArray + i + 1) - *(pMergeArray->xArray + i);
		tempDaltY = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i + 1) - *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i) - *(pMergeArray->yArray + i + 1);
		tempMinY = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i + 1);
		ARITH_INFO("i = %d, tempDaltX = %d, tempDaltY = %d, tempMinY = %d\r\n",i,tempDaltX,tempDaltY,tempMinY);

		if((tempDaltX >= x_axis_run_min_threshold) && (tempDaltY > g_yAxisDaltMinThreshold && tempDaltY > tempMinY/5 && tempDaltY < tempMinY*5))
		{
			ARITH_INFO("Run: runCount = %d, runx = %d,runy = %d, mergeX[%d] = %d, mergeY[%d] = %d, mergeX[%d] = %d, mergeY[%d] = %d\r\n",
				runCount, runCount > 0 ? *(pRunXArray + runCount - 1): 0, runCount > 0 ? *(pRunXArray + runCount - 1): 0,
				i,*(pMergeArray->xArray + i),i,*(pMergeArray->yArray + i),i+1,*(pMergeArray->xArray + i + 1),i+1,*(pMergeArray->yArray + i + 1));
			if(runCount == 0 || *(pRunXArray + runCount - 1) != *(pMergeArray->xArray + i))
			{
				*(pRunXArray+runCount) = *(pMergeArray->xArray + i);
				*(pRunYArray+runCount) = *(pMergeArray->yArray + i);
				runCount++;
			}

			if(runCount == 0 || *(pRunXArray + runCount - 1) != *(pMergeArray->xArray + i + 1))
			{
				*(pRunXArray+runCount) = *(pMergeArray->xArray + i + 1);
				*(pRunYArray+runCount) = *(pMergeArray->yArray + i + 1);
				runCount++;
			}

			if((tempDaltX >= x_axis_walk_min_threshold) && (tempDaltY > g_yAxisDaltMinThreshold && tempDaltY > tempMinY/3 && tempDaltY < tempMinY*5))
		//	if(tempDaltX >= X_AXIS_WALK_MIN_THRESHOLD)
			{
				ARITH_INFO("Walk: walkCount = %d, walkx = %d, walky = %d, mergeX[%d] = %d, mergeY[%d] = %d, mergeX[%d] = %d, mergeY[%d] = %d\r\n",
					walkCount, walkCount > 0 ? *(pWalkXArray + walkCount - 1): 0, walkCount > 0 ? *(pWalkYArray + walkCount - 1): 0,
					i,*(pMergeArray->xArray + i),i,*(pMergeArray->yArray + i),i+1,*(pMergeArray->xArray + i + 1),i+1,*(pMergeArray->yArray + i + 1));
				if(walkCount == 0 || *(pWalkXArray + walkCount - 1) != *(pMergeArray->xArray + i))
				{
					*(pWalkXArray+walkCount) = *(pMergeArray->xArray + i);
					*(pWalkYArray+walkCount) = *(pMergeArray->yArray + i);
					walkCount++;
				}

				if(walkCount == 0 || *(pWalkXArray + walkCount - 1) != *(pMergeArray->xArray + i + 1))
				{
					*(pWalkXArray+walkCount) = *(pMergeArray->xArray + i + 1);
					*(pWalkYArray+walkCount) = *(pMergeArray->yArray + i + 1);
					walkCount++;
				}
			}
		}
		else if((tempDaltX < x_axis_run_min_threshold) && (tempDaltY > g_yAxisDaltMinThreshold && tempDaltY > tempMinY/5 && tempDaltY < tempMinY*5))
		{
			invalidXCount++;
		}
		else if(tempDaltX >= x_axis_run_min_threshold)
		{
			invalidXYCount++;
		}
	}

#if ARI_MODULE_LOG_EN
	ARITH_INFO("\r\n invalidXCount = %d\r\nWalk Temp Array %d items: \r\n",invalidXCount, walkCount);
	for(i = 0; i < walkCount; i++)
	{
		ARITH_INFO("[%d :% d], ",*(pWalkXArray+i),*(pWalkYArray+i));
	}
	ARITH_INFO("\r\nRun Temp Array %d items: \r\n", walkCount);
	for(i = 0; i < runCount; i++)
	{
		ARITH_INFO("[%d :% d], ",*(pRunXArray+i),*(pRunYArray+i));
	}
	ARITH_INFO("\r\n");
#endif


	if((walkCount >= 2 && (runCount - 1) >= 3*(walkCount - 1)) ||
		(walkCount < 2))
	{
		*pSportMode = 1;

		if((invalidXCount <= runCount*3/2) && (invalidXCount + invalidXYCount <= runCount*2))
			steps = steps_matchArray_verfy(pRunXArray,pRunYArray,runCount,1);
	}
	else
	{
		*pSportMode = 0;

		steps = steps_matchArray_verfy(pWalkXArray,pWalkYArray,walkCount,0);
	}

	return steps;
}

unsigned char deal_raw_data(unsigned short *yArray, unsigned short length, unsigned char* pSportMode,unsigned char hand)
{
	unsigned char steps = 0;
	unsigned short *firYArray = NULL;
	signed short *derivativeYArray = NULL;
	signed short *smoothYArray = NULL;
	unsigned short i=0;
	unsigned int sum=0;
	XYDataArray peaksArray = {0};
	XYDataArray valleysArray = {0};
	XYDataArray mergeArray = {0};
	
	unsigned short buf_size_256_1[BLE_ARTH_BUF_SIZE]={0};
	signed short buf_size_256_2[BLE_ARTH_BUF_SIZE]={0};
	signed short buf_size_256_3[BLE_ARTH_BUF_SIZE]={0};

	firYArray = (unsigned short*)buf_size_256_1;
	memset(firYArray,0,length*2);
	FIRProcess(yArray,length,firYArray);// 滤波

	yMax= *(firYArray+0);
	yMin= *(firYArray+0);
	for(i=0;i<length;i++)
	{
		if(*(firYArray+i) > yMax)
			yMax = *(firYArray+i);

		if(*(firYArray+i) < yMin)
			yMin = *(firYArray+i);

		sum += *(firYArray+i);
	}
	yAverage = sum / length;
	MY_QPRINTF("yAverage=%d,	yMax=%d,	yMin=%d,	\r\n",yAverage,yMax,yMin);

	if(yAverage < 17000)
	{
		x_axis_run_min_threshold 	= X_AXIS_RUN_MIN_THRESHOLD+2;
		x_axis_walk_min_threshold 	= X_AXIS_WALK_MIN_THRESHOLD+2;
		g_yAxisDaltMinThreshold		= 850;
		MY_QPRINTF("MODE 0000\r\n");
	}
	if(yAverage < 18000)
	{
		x_axis_run_min_threshold 	= X_AXIS_RUN_MIN_THRESHOLD+1;
		x_axis_walk_min_threshold 	= X_AXIS_WALK_MIN_THRESHOLD+1;
		g_yAxisDaltMinThreshold		= 950;
		MY_QPRINTF("MODE 1111\r\n");
	}
	else if(yAverage < 20000)
	{
		x_axis_run_min_threshold 	= X_AXIS_RUN_MIN_THRESHOLD;
		x_axis_walk_min_threshold 	= X_AXIS_WALK_MIN_THRESHOLD;
		g_yAxisDaltMinThreshold		= 1000;
		MY_QPRINTF("MODE 2222\r\n");
	}
	else if(yAverage < 30000)
	{
		if(yMax - yMin > 10000)
		{
			x_axis_run_min_threshold 	= X_AXIS_RUN_MIN_THRESHOLD-2;
			x_axis_walk_min_threshold 	= X_AXIS_WALK_MIN_THRESHOLD-2;
			g_yAxisDaltMinThreshold		= 1100;
			MY_QPRINTF("MODE 3333\r\n");
		}
		else
		{
			x_axis_run_min_threshold 	= X_AXIS_RUN_MIN_THRESHOLD-1;
			x_axis_walk_min_threshold 	= X_AXIS_WALK_MIN_THRESHOLD-1;
			g_yAxisDaltMinThreshold		= 1100;
			MY_QPRINTF("MODE 4444\r\n");
		}
	}
	else
	{
		if(yMax > 48000 && yMin > 30000)
		{
			x_axis_run_min_threshold 	= X_AXIS_RUN_MIN_THRESHOLD-6;
			x_axis_walk_min_threshold 	= X_AXIS_WALK_MIN_THRESHOLD-6;
			g_yAxisDaltMinThreshold		= 1000;
			MY_QPRINTF("MODE 5555\r\n");
		}
		else if(yMax >30000)
		{
			x_axis_run_min_threshold 	= X_AXIS_RUN_MIN_THRESHOLD-5;
			x_axis_walk_min_threshold 	= X_AXIS_WALK_MIN_THRESHOLD-5;
			g_yAxisDaltMinThreshold		= 1000;
			MY_QPRINTF("MODE 6666\r\n");
		}
		else
		{
			if(yMax - yMin > 10000)
			{
				x_axis_run_min_threshold 	= X_AXIS_RUN_MIN_THRESHOLD-4;
				x_axis_walk_min_threshold 	= X_AXIS_WALK_MIN_THRESHOLD-3;
				g_yAxisDaltMinThreshold		= 1200;
				MY_QPRINTF("MODE 7777\r\n");
			}
			else
			{
				x_axis_run_min_threshold 	= X_AXIS_RUN_MIN_THRESHOLD-3;
				x_axis_walk_min_threshold 	= X_AXIS_WALK_MIN_THRESHOLD-2;
				g_yAxisDaltMinThreshold		= 1150;
				MY_QPRINTF("MODE 8888\r\n");
			}
		}
		
	}

	derivativeYArray = (signed short*)buf_size_256_2;
	memset(derivativeYArray,0,length*2);
	derivativeFunc(length,yArray,derivativeYArray);// 求导

	smoothYArray = (signed short*)buf_size_256_3;
	memset(smoothYArray,0,length*2);
	smoothfunc(length,0,derivativeYArray,smoothYArray);//平滑


	peaksArray.xArray = (unsigned short*)buf_size_256_1;
	peaksArray.yArray = (unsigned short*)(buf_size_256_1+1*(BLE_ARTH_BUF_SIZE/2));
	valleysArray.xArray = (unsigned short*)(buf_size_256_2);
	valleysArray.yArray = (unsigned short*)(buf_size_256_2+1*(BLE_ARTH_BUF_SIZE/2));
	memset(peaksArray.xArray,0,length);
	memset(peaksArray.yArray,0,length);
	memset(valleysArray.xArray,0,length);
	memset(valleysArray.yArray,0,length);

	peaksArray.num = 0;
	FindPeaks(yArray,length,0,smoothYArray,&peaksArray);// 找波峰
	if(peaksArray.num == 0)
	{
		goto MEM_FREE;
	}

	valleysArray.num = 0;
	FindValleys(yArray,length,0,smoothYArray,&valleysArray);
	if(valleysArray.num == 0)
	{
		goto MEM_FREE;
	}

	mergeArray.xArray = (unsigned short*)buf_size_256_3;
	mergeArray.yArray = (unsigned short*)(buf_size_256_3+(BLE_ARTH_BUF_SIZE/2));
	peaks_valleys_Merge(&peaksArray,&valleysArray,&mergeArray);//合并
	
	peaksValleys_mergeArray_arrange(&mergeArray);

	if(mergeArray.num <= 1)
		goto MEM_FREE;
	
	steps = peaks_valleys_final_match(&mergeArray, pSportMode);
MEM_FREE:
	return steps;
}

#elif USE_ARITHMETIC_FROM == MODIFY_WUFAN_ARITHMETIC

unsigned char g_steps_start_flag = 1;

static unsigned short g_yAxisDaltMinThreshold = Y_AXIS_DALT_DAY_MIN_THRESHOLD_L;

void Arithmetic_Set_YAxis_Min_Threshold(unsigned short data) 
{
	g_yAxisDaltMinThreshold = data;
}

static void FIRProcess(unsigned short *InData, unsigned short length, unsigned short *OutData,unsigned char firSmoothInterval)
{	
	unsigned int sum = 0; 
	unsigned short i;
	unsigned short  j = 0;
	
	//for(unsigned char k=0;k<2;k++)
	{
		for(i = 0;i < length - firSmoothInterval;i++)
		{	
			sum = 0;
			for(j = 0;j < firSmoothInterval;j++)
			{
				sum += InData[i+j];  
			} 
			InData[i] = sum/firSmoothInterval;
		}
		for(i = length - firSmoothInterval; i < length; i++)
		{	
			sum = 0;
			for(j = 0;j < firSmoothInterval;j++)
			{
				sum += InData[i-j];  
			}
			InData[i] = sum/firSmoothInterval;
		}
	}

}

static void derivativeFunc(unsigned short length, unsigned short *src, signed short *dist)
{
	unsigned short i = 0;
	if(length > 2)
	{
		#if 1
		*dist = *(src+1) - *src;
		*(dist + length - 1) = *(src + length - 1) - *(src + length - 2);

		for(i = 1; i < length - 2; i++)
			*(dist + i) = (*(src + i + 1) - *(src + i - 1))/2;
		#else
		for(i = 0; i < length - 2; i++)
			*(dist + i) = *(src + i + 1) - *(src + i);
		#endif
	}
	else
	{
		for(i = 0; i < length; i++)
			*(dist + i) = *(src + i);
	}
	return;
}

static void smoothfunc(unsigned short length, unsigned short ends,signed short* src, signed short* dist)
{
	unsigned short sw = SMOOTH_WIDTH;
	unsigned short i = 0, j = 0, halfSW = 0, startPoint = 0;
	signed int sumVal = 0;
	if(length <= sw)
		sw = length/2;
	halfSW = sw/2;

	if(length <= 2)
	{
		for(i = 0; i < length; i++)
			*(dist + i) = *(src + i);
		return;
	}
	
	for(i = 0; i < sw; i++)
		sumVal += *(src+i);
		
	for(i = 0; i < length - sw -1; i++)
	{
		*(dist + i +halfSW - 2) = (signed short)sumVal;
		sumVal -= *(src + i);
		sumVal += *(src + i + sw - 1);
	}
	
	sumVal = 0;
	for(j = length-sw -1; j < length; j++)
		sumVal += *(src + j);
		
	*(dist + i +halfSW - 1) = (signed short)sumVal;
	for(j = 0; j < length; j++)
	{
		*(dist + j) = *(dist + j)/sw;
	}
	
	if(ends == 1)
	{
		startPoint = sw/2;
		*dist = (*src + *(src+1))/2;
		for(i = 1; i < startPoint; i++)
		{
			sumVal = 0;
			for(j = 0; j < 2*i - 1; j++)
				sumVal += *(src+j);
			*(dist + i) = (signed short)(sumVal/j);
			
			sumVal = 0;
			for(j = length - 2*i + 1; j < length; j++)
				sumVal += *(src+j);
			*(dist + length - i - 1) = (signed short)(sumVal/(2*i - 1));
		}
		*(dist + length - 1) = (*(src + length - 1) + *(src + length - 2))/2;
	}

	return;
}


static unsigned short FindPeaks(unsigned short *y, unsigned short length,unsigned short AmpThreshold, signed short* smoothYArray, XYDataArray* pPeaksArray)
{
	unsigned short j = 0, peakCount = 0, startPos = 0;
	unsigned short smoothwidth = SMOOTH_WIDTH;
	if(length <= smoothwidth)
		smoothwidth = length/2;

	if(length <= 2)
	{
		return length;
	}
	
	
    if(smoothwidth > 1)
		startPos = 2*(smoothwidth/2) - 1;
	else
		startPos = 1;
	
    for (j = startPos-1 ; j < length - smoothwidth - 1 ;j++)
    {
        if((*(smoothYArray+j) >= 0) && (*(smoothYArray+j+1) < 0))
        {
            if(*(y + j) > AmpThreshold)   
            {
				*(pPeaksArray->xArray + peakCount) = j;
				*(pPeaksArray->yArray + peakCount) = *(y + j);
				peakCount++;
            } 
        }
    }
	pPeaksArray->num = peakCount;
	return peakCount;
}


static unsigned short FindValleys(unsigned short *y,unsigned short length,unsigned short AmpThreshold, signed short* smoothYArray, XYDataArray* pValleysArray)
{
	unsigned short j = 0, valleyCount = 0, startPos = 0;
	unsigned short smoothwidth = SMOOTH_WIDTH;
	if(length <= smoothwidth)
		smoothwidth = length/2;

	if(length <= 2)
	{
		return length;
	}
	
    if(smoothwidth > 1)
		startPos = 2*(smoothwidth/2) - 1;
	else
		startPos = 1;

    for (j = startPos-1 ; j < length - smoothwidth - 1 ;j++)
    {
        if((*(smoothYArray+j) < 0) && (*(smoothYArray+j+1) >= 0))
        {       
            if(*(y + j) > AmpThreshold)   
            {                 
				*(pValleysArray->xArray + valleyCount) = j;
				*(pValleysArray->yArray + valleyCount) = *(y + j);
				valleyCount++;
                    
            }
        }
    }
	pValleysArray->num = valleyCount;
	return valleyCount;
}


static void peaks_valleys_Merge(XYDataArray* pPeaksArray, XYDataArray* pValleysArray, XYDataArray* pMergeArray)
{
	unsigned short i = 0, count = 0, peakPos = 0, valleyPos = 0;
	
	memset(pMergeArray->xArray,0,BLE_ARTH_BUF_SIZE);
	memset(pMergeArray->yArray,0,BLE_ARTH_BUF_SIZE);
	pMergeArray->num = 0;
	
	while(i < (pPeaksArray->num + pValleysArray->num))
	{
		if((peakPos < pPeaksArray->num) && (valleyPos < pValleysArray->num))
		{
			if(*(pPeaksArray->xArray + peakPos) < *(pValleysArray->xArray + valleyPos))
			{
				*(pMergeArray->xArray + count) = *(pPeaksArray->xArray + peakPos);
				*(pMergeArray->yArray + count) = *(pPeaksArray->yArray + peakPos);
				peakPos++;
				count++;
				i++;
			}
			else if(*(pPeaksArray->xArray + peakPos) > *(pValleysArray->xArray + valleyPos))
			{
				*(pMergeArray->xArray + count) = *(pValleysArray->xArray + valleyPos);
				*(pMergeArray->yArray + count) = *(pValleysArray->yArray + valleyPos);
				valleyPos++;
				count++;
				i++;
			}
			else 
			{
				*(pMergeArray->xArray + count) = *(pPeaksArray->xArray + peakPos);
				*(pMergeArray->yArray + count) = *(pPeaksArray->yArray + peakPos);
				peakPos++;
				valleyPos++;
				count++;
				i+=2;
			}
		}
		else if(peakPos >= pPeaksArray->num)
		{
			*(pMergeArray->xArray + count) = *(pValleysArray->xArray + valleyPos);
			*(pMergeArray->yArray + count) = *(pValleysArray->yArray + valleyPos);
			valleyPos++;
			count++;
			i++;
		}
		else
		{
			*(pMergeArray->xArray + count) = *(pPeaksArray->xArray + peakPos);
			*(pMergeArray->yArray + count) = *(pPeaksArray->yArray + peakPos);
			peakPos++;
			count++;
			i++;
		}
	}

	pMergeArray->num = count;

}


static void peaksValleys_mergeArray_arrange(XYDataArray* pMergeArray)
{
	unsigned short i = 0, j = 0,daltX = 0, daltY = 0, minYVal = 0, count = 0, beginPos = 0, find = 0;

	unsigned short arrangeXArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short arrangeYArray[BLE_ARTH_BUF_SIZE/2];

	unsigned char y_precent = 0;

	if(g_steps_start_flag)
		y_precent = 16;
	else
		y_precent = 8;

	for(i = 0; i < pMergeArray->num - 1; i++)
	{
		daltX = *(pMergeArray->xArray + i + 1) - *(pMergeArray->xArray + i);
		daltY = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i + 1) - *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i)  - *(pMergeArray->yArray + i + 1) ;
		minYVal = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i + 1);
		MY_QPRINTF("daltX=%d,daltY=%d,minYVal=%d,y_precent=%d,\r\n",daltX,daltY,minYVal,y_precent);
		if(daltX <= 5 && daltY <= minYVal/y_precent)//minYVal/8
		{
			if(find == 0)
			{
				find = 1;
				beginPos = i;
			}
		}
		else
		{
			if(find == 1)
			{
				unsigned int sumX = 0, sumY = 0;
				unsigned short avgX = 0, avgY = 0;
				find = 0;
				for(j = beginPos; j <= i; j++)
				{
					sumX += *(pMergeArray->xArray + j);
					sumY += *(pMergeArray->yArray + j);
				}
			
				avgX = sumX/(i - beginPos+1);
				avgY = sumY/(i - beginPos+1);

				*(arrangeXArray + count) = avgX;
				*(arrangeYArray + count) = avgY;
				count++;
			}
			else
			{
				*(arrangeXArray + count) = *(pMergeArray->xArray + i);
				*(arrangeYArray + count) = *(pMergeArray->yArray + i);
				count++;
			}
		}
	}

	if(find == 1)
	{
		unsigned int sumX = 0, sumY = 0;
		unsigned short avgX = 0, avgY = 0;
		find = 0;
		for(j = beginPos; j <= i; j++)
		{
			sumX += *(pMergeArray->xArray + j);
			sumY += *(pMergeArray->yArray + j);
		}
	
		avgX = sumX/(i - beginPos+1);
		avgY = sumY/(i - beginPos+1);

		*(arrangeXArray + count) = avgX;
		*(arrangeYArray + count) = avgY;
		count++;
	}
	else
	{
		if(i == pMergeArray->num - 1)
		{
			*(arrangeXArray + count) = *(pMergeArray->xArray + i);
			*(arrangeYArray + count) = *(pMergeArray->yArray + i);
			count++;
		}
	}
	memset(pMergeArray->xArray, 0 ,pMergeArray->num*2);
	memset(pMergeArray->yArray, 0 ,pMergeArray->num*2);
	pMergeArray->num = count;
	memcpy(pMergeArray->xArray, arrangeXArray ,count*2);
	memcpy(pMergeArray->yArray, arrangeYArray ,count*2);
}


static unsigned char steps_matchArray_verfy(unsigned short* xArray, unsigned short* yArray, unsigned short arrayNum, unsigned char sportMode,unsigned char hand)
{
	unsigned short i = 0, xDaltMin = 0, invalidTotalCount = 0, disconnectCount = 0, onceMaxNum = 0, validTotalCount = 0, steps = 0, find = 0, daltX = 0, beginPos = 0;

	if(g_steps_start_flag)
	{
		if(hand)//right
		{
			if(sportMode) 
				xDaltMin = X_AXIS_RUN_MIN_THRESHOLD_MIN_R;
			else 
				xDaltMin = X_AXIS_WALK_MIN_THRESHOLD_MIN_R;
		}
		else//left
		{
			if(sportMode) 
				xDaltMin = X_AXIS_RUN_MIN_THRESHOLD_MIN_L;
			else 
				xDaltMin = X_AXIS_WALK_MIN_THRESHOLD_MIN_L;
		}
		
	}
	else
	{
		if(hand)//right
		{
			if(sportMode) 
				xDaltMin = X_AXIS_RUN_MIN_THRESHOLD_R;
			else 
				xDaltMin = X_AXIS_WALK_MIN_THRESHOLD_R;
		}
		else//left
		{
			if(sportMode) 
				xDaltMin = X_AXIS_RUN_MIN_THRESHOLD_L;
			else 
				xDaltMin = X_AXIS_WALK_MIN_THRESHOLD_L;
		}
		
	}

	while(i < arrayNum - 1)
	{
		daltX = *(xArray + i + 1) - *(xArray + i);
		if(daltX >= xDaltMin)
		{
			if(find == 0)
			{
				find = 1;
				beginPos = i;	
			}
			validTotalCount++;
		}
		else
		{
			if(find == 1)
			{
				find = 0;
				disconnectCount++;
				if(i - beginPos > onceMaxNum)
					onceMaxNum = i - beginPos;
			}
			invalidTotalCount++;
		}
		i++;
	}

	if(find == 1)
	{
		if(i - beginPos > onceMaxNum)
			onceMaxNum = i - beginPos;
	}

 	if((validTotalCount > invalidTotalCount) && (validTotalCount > disconnectCount))
 	{
			steps = onceMaxNum;	
	}

	if(steps > 5)
		return 0;
	
	if(sportMode)
		steps = steps > 4 ? 4 : steps;
	else
		steps = steps > 2 ? 2 : steps;
	return steps;
}


static unsigned char peaks_valleys_final_match(XYDataArray* pMergeArray, unsigned char* pSportMode,unsigned char hand,unsigned short avg_acc_value)
{
	unsigned short tempDaltX = 0, tempDaltY = 0, tempMinY = 0;
	signed short i = 0, walkCount = 0, runCount = 0, invalidXCount = 0, invalidXYCount = 0, steps = 0;

	unsigned short pWalkXArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short pWalkYArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short pRunXArray[BLE_ARTH_BUF_SIZE/2];
	unsigned short pRunYArray[BLE_ARTH_BUF_SIZE/2];

	unsigned char x_run_min = 0,x_walk_min = 0,y_run_precent=0,y_walk_present=0;
	unsigned short y_high_value = 0;
	
	if(g_steps_start_flag)
	{
		if(hand)//right
		{
			x_run_min 		= X_AXIS_RUN_MIN_THRESHOLD_MIN_R;
			x_walk_min 		= X_AXIS_WALK_MIN_THRESHOLD_MIN_R;
			y_run_precent 	= 14;
			y_walk_present 	= 9;
			y_high_value 	= Y_AXIS_DALT_DAY_MIN_THRESHOLD_MIN_R;
		}
		else
		{
			x_run_min 		= X_AXIS_RUN_MIN_THRESHOLD_MIN_L;
			x_walk_min 		= X_AXIS_WALK_MIN_THRESHOLD_MIN_L;
			y_run_precent 	= 18;
			y_walk_present	= 13;
			y_high_value 	= Y_AXIS_DALT_DAY_MIN_THRESHOLD_MIN_L;
		}
		
	}
	else
	{	
		if(hand)//right
		{
			x_run_min 		= X_AXIS_RUN_MIN_THRESHOLD_R;
			x_walk_min 		= X_AXIS_WALK_MIN_THRESHOLD_R;
			y_run_precent 	= 5;
			y_walk_present 	= 3;
			y_high_value 	= Y_AXIS_DALT_DAY_MIN_THRESHOLD_R;
		}
		else//left
		{
			x_run_min 		= X_AXIS_RUN_MIN_THRESHOLD_L;
			x_walk_min 		= X_AXIS_WALK_MIN_THRESHOLD_L;
			y_run_precent 	= 8;
			y_walk_present 	= 5;
			y_high_value 	= Y_AXIS_DALT_DAY_MIN_THRESHOLD_L;
		}
		
	}
	Arithmetic_Set_YAxis_Min_Threshold(y_high_value);
		
	for(i = 0; i < pMergeArray->num - 1; i++)
	{
		tempDaltX = *(pMergeArray->xArray + i + 1) - *(pMergeArray->xArray + i);
		tempDaltY = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i + 1) - *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i) - *(pMergeArray->yArray + i + 1);
		tempMinY = *(pMergeArray->yArray + i + 1) >= *(pMergeArray->yArray + i) ? *(pMergeArray->yArray + i) : *(pMergeArray->yArray + i + 1);
		MY_QPRINTF("tempDaltX=%d,tempDaltY=%d,tempMinY=%d,g_yAxisDaltMinThreshold=%d,\r\n",tempDaltX,tempDaltY,tempMinY,g_yAxisDaltMinThreshold);
		if((tempDaltX >= x_run_min) && (tempDaltY > g_yAxisDaltMinThreshold && tempDaltY > tempMinY/y_run_precent && tempDaltY < tempMinY*5))//tempMinY/5
		{
			if(runCount == 0 || *(pRunXArray + runCount - 1) != *(pMergeArray->xArray + i))
			{
				*(pRunXArray+runCount) = *(pMergeArray->xArray + i);
				*(pRunYArray+runCount) = *(pMergeArray->yArray + i);
				runCount++;
			}

			if(runCount == 0 || *(pRunXArray + runCount - 1) != *(pMergeArray->xArray + i + 1))
			{
				*(pRunXArray+runCount) = *(pMergeArray->xArray + i + 1);
				*(pRunYArray+runCount) = *(pMergeArray->yArray + i + 1);
				runCount++;
			}

			if((tempDaltX >= x_walk_min) && (tempDaltY > g_yAxisDaltMinThreshold && tempDaltY > tempMinY/y_walk_present && tempDaltY < tempMinY*5))//tempMinY/3
			//if(tempDaltX >= X_AXIS_WALK_MIN_THRESHOLD)
			{
				if(walkCount == 0 || *(pWalkXArray + walkCount - 1) != *(pMergeArray->xArray + i))
				{
					*(pWalkXArray+walkCount) = *(pMergeArray->xArray + i);
					*(pWalkYArray+walkCount) = *(pMergeArray->yArray + i);
					walkCount++;
				}

				if(walkCount == 0 || *(pWalkXArray + walkCount - 1) != *(pMergeArray->xArray + i + 1))
				{
					*(pWalkXArray+walkCount) = *(pMergeArray->xArray + i + 1);
					*(pWalkYArray+walkCount) = *(pMergeArray->yArray + i + 1);
					walkCount++;
				}

			}

		}
		else if((tempDaltX < x_run_min) && (tempDaltY > g_yAxisDaltMinThreshold && tempDaltY > tempMinY/5 && tempDaltY < tempMinY*5))
		{
			invalidXCount++;
		}
		else if(tempDaltX >= x_run_min)
		{
			invalidXYCount++;
		}
	}
#if 0
	unsigned int ySum = 0;
	for(i = 0; i < runCount; i++)
	{
		ySum += *(pRunYArray+i);
	}
	ySum /= runCount;
#endif
	
	//if((walkCount >= 2 && (runCount - 1) >= 3*(walkCount - 1)) ||
		//(walkCount < 2))
	#if 0
	if(runCount > 0)
	{
		if(ySum > 27000)
		{
			*pSportMode = 1;
			steps = steps_matchArray_verfy(pRunXArray,pRunYArray,runCount,0);
		}
		else
		{
			*pSportMode = 0;
			if(walkCount > 0)
				steps = steps_matchArray_verfy(pWalkXArray,pWalkYArray,walkCount,0);
			else
				steps = steps_matchArray_verfy(pRunXArray,pRunYArray,runCount,0);
			
		}

		
	}
	#else
	if(avg_acc_value > 11000)
		g_steps_start_flag = 0;
	else
		g_steps_start_flag = 1; 
	MY_QPRINTF("ySum=%d,walkCount=%d,runCount=%d,\r\n",avg_acc_value,walkCount,runCount);
	if((avg_acc_value > 16000) && ( (avg_acc_value > 30000 && runCount >= 2) || (walkCount < 2) || (runCount - 1) >= 3*(walkCount - 1) ) )
	{
		*pSportMode = 1;
		if((invalidXCount <= runCount*3/2) && (invalidXCount + invalidXYCount <= runCount*2))
		{
			steps = steps_matchArray_verfy(pRunXArray,pRunYArray,runCount,1,hand);
		}
	}
	else
	{
		*pSportMode = 0;
		steps = steps_matchArray_verfy(pWalkXArray,pWalkYArray,walkCount,0,hand);
	}
	#endif
	return steps;
}


unsigned char deal_raw_data(unsigned short *yArray, unsigned short length, unsigned char* pSportMode,unsigned char hand)
{
	unsigned char steps = 0;
	unsigned short *firYArray = NULL;
	signed short *derivativeYArray = NULL;
	signed short *smoothYArray = NULL;
	unsigned short avg_acc_value = 0,i=0;
	unsigned int sum = 0;
	XYDataArray peaksArray = {0};
	XYDataArray valleysArray = {0};
	XYDataArray mergeArray = {0};
	
	unsigned short buf_size_256_1[BLE_ARTH_BUF_SIZE]={0};
	signed short buf_size_256_2[BLE_ARTH_BUF_SIZE]={0};
	signed short buf_size_256_3[BLE_ARTH_BUF_SIZE]={0};

	firYArray = (unsigned short*)buf_size_256_1;
	memset(firYArray,0,length*2);
	FIRProcess(yArray,length,firYArray,FIR_SMOOTH_INTERVAL);// 滤波

	sum  = 0;
	for(i=0;i<length;i++)
		sum += yArray[i];
	avg_acc_value = sum / length;

	derivativeYArray = (signed short*)buf_size_256_2;
	memset(derivativeYArray,0,length*2);
	derivativeFunc(length,yArray,derivativeYArray);// 求导

	smoothYArray = (signed short*)buf_size_256_3;
	memset(smoothYArray,0,length*2);
	smoothfunc(length,0,derivativeYArray,smoothYArray);//平滑


	peaksArray.xArray = (unsigned short*)buf_size_256_1;
	peaksArray.yArray = (unsigned short*)(buf_size_256_1+1*(BLE_ARTH_BUF_SIZE/2));
	valleysArray.xArray = (unsigned short*)(buf_size_256_2);
	valleysArray.yArray = (unsigned short*)(buf_size_256_2+1*(BLE_ARTH_BUF_SIZE/2));
	memset(peaksArray.xArray,0,length);
	memset(peaksArray.yArray,0,length);
	memset(valleysArray.xArray,0,length);
	memset(valleysArray.yArray,0,length);



	peaksArray.num = 0;
	FindPeaks(yArray,length,0,smoothYArray,&peaksArray);// 找波峰
	if(peaksArray.num == 0)
	{
		goto MEM_FREE;
	}

	valleysArray.num = 0;
	FindValleys(yArray,length,0,smoothYArray,&valleysArray);
	if(valleysArray.num == 0)
	{
		goto MEM_FREE;
	}

	mergeArray.xArray = (unsigned short*)buf_size_256_3;
	mergeArray.yArray = (unsigned short*)(buf_size_256_3+(BLE_ARTH_BUF_SIZE/2));
	peaks_valleys_Merge(&peaksArray,&valleysArray,&mergeArray);//合并
	
	peaksValleys_mergeArray_arrange(&mergeArray);


	
	if(mergeArray.num <= 1)
		goto MEM_FREE;
	
	steps = peaks_valleys_final_match(&mergeArray, pSportMode,hand,avg_acc_value);

MEM_FREE:
	return steps;
}
#endif
#endif

