/**
 * @file mq2.cpp
 * @author Evandro Teixeira
 * @brief 
 * @version 0.1
 * @date 31-08-2020
 * 
 * @copyright Copyright (c) 2020
 * @note based on the algorithm https://os.mbed.com/users/azazeal88/code/MQ2/
 */
#include "mq2.h"
#include <math.h>
#include "driver/gpio.h"
#include "driver/adc.h"


#define RL_VALUE                    1       //define the load resistance on the board, in kilo ohms
#define RO_DEFAULT                  10      //Ro is initialized to 10 kilo ohms
#define RO_CLEAN_AIR_FACTOR         9.83f   //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO, which is derived from the chart in datasheet
#define CALIBARAION_SAMPLE_TIMES    5       //define how many samples you are going to take in the calibration phase
#define CALIBRATION_SAMPLE_INTERVAL 50      //define the time interal(in milisecond) between each samples
#define READ_SAMPLE_INTERVAL        50      //define how many samples you are going to take in normal operation
#define READ_SAMPLE_TIMES           5       //define the time interal(in milisecond) between each samples
#define NUMBER_PIN_ADC              15

/**
 * @brief 
 * 
 */
typedef enum
{
    Channel_ADC1 = 0,
    Channel_ADC2,
    Channel_MAX
}ChannelADC_t;

/**
 * @brief 
 * 
 */
typedef struct  
{
    uint8_t gpio;
    ChannelADC_t channel;
    uint8_t pin_adc;
}PinADC_t;

/**
 * @brief The curves
 * 
 */
static float LPGCurve[3]   = {2.3f,0.21f,-0.47f};
static float COCurve[3]    = {2.3f,0.72f,-0.34f};
static float SmokeCurve[3] = {2.3f,0.53f,-0.44f};

/**
 * @brief ESP32 DevKit V1 ADC pin Map
 * 
 */
const static PinADC_t ADC_PIN_MAP[NUMBER_PIN_ADC] =
{
/*  GPIO | Channel ADC |    pin ADC     |  Index */ 
    { 36,  Channel_ADC1, ADC1_CHANNEL_0 }, //  0
    { 39,  Channel_ADC1, ADC1_CHANNEL_3 }, //  1
    { 34,  Channel_ADC1, ADC1_CHANNEL_6 }, //  2
    { 35,  Channel_ADC1, ADC1_CHANNEL_7 }, //  3
    { 32,  Channel_ADC1, ADC1_CHANNEL_4 }, //  4
    { 33,  Channel_ADC1, ADC1_CHANNEL_5 }, //  5
    { 25,  Channel_ADC2, ADC2_CHANNEL_8 }, //  6
    { 26,  Channel_ADC2, ADC2_CHANNEL_9 }, //  7
    { 27,  Channel_ADC2, ADC2_CHANNEL_7 }, //  8
    { 14,  Channel_ADC2, ADC2_CHANNEL_6 }, //  9
    { 12,  Channel_ADC2, ADC2_CHANNEL_5 }, //  10
    { 13,  Channel_ADC2, ADC2_CHANNEL_4 }, //  11
    { 15,  Channel_ADC2, ADC2_CHANNEL_3 }, //  12
    {  2,  Channel_ADC2, ADC2_CHANNEL_2 }, //  13
    {  4,  Channel_ADC2, ADC2_CHANNEL_0 }, //  14
};

/**
 * @brief Construct a new mq2::mq2 object
 * 
 * @param gpio 
 */
mq2::mq2(uint8_t io) 
{
    R0 = RO_DEFAULT;
    get_index(io,&index);
}

/**
 * @brief Destroy the mq2::mq2 object
 * 
 */
mq2::~mq2()
{
}

/**
 * @brief 
 * 
 */
void mq2::begin(void)
{
    //adc_init(gpio);
    //get_index(gpio);
    //adc1_config_width(ADC_WIDTH_BIT_12);
    //adc1_config_channel_atten((adc1_channel_t)ADC1_CHANNEL_0,ADC_ATTEN_DB_11);
    switch(ADC_PIN_MAP[index].channel)
    {
        case Channel_ADC1:
            adc1_config_width(ADC_WIDTH_BIT_12);
            adc1_config_channel_atten((adc1_channel_t)ADC1_CHANNEL_0,ADC_ATTEN_DB_11);
        break;
        case Channel_ADC2:
            adc2_config_channel_atten((adc2_channel_t)ADC_PIN_MAP[index].pin_adc, ADC_ATTEN_DB_11 );
        break;
        case Channel_MAX:
        break;
    }

    R0 = Calibration();
}

/**
 * @brief 
 * 
 * @param type 
 * @return float 
 */
float mq2::get(MQ2GasType_t type)
{
    switch (type)
    {
        case GAS_LPG: 
            return GetGasPercentage(Read()/R0,GAS_LPG);
        break;
        case GAS_CO: 
            return GetGasPercentage(Read()/R0,GAS_CO);
        break;
        case GAS_SMOKE: 
            return GetGasPercentage(Read()/R0,GAS_SMOKE);
        break;
        default: 
            return -1.0;
        break;
    }
}

/**
 * @brief 
 * 
 * @return float 
 */
float mq2::Calibration(void)
{
    uint8_t i = 0;
    float val=0;
    uint16_t adc = 0;
    for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) 
    {                                        
        //take multiple samples
        //adc_get_value(gpio, &adc);
        //adc = adc1_get_raw((adc1_channel_t)ADC1_CHANNEL_0);
        adc = get_value_adc(index);
        val += ResistanceCalculation(adc);
        delay(CALIBRATION_SAMPLE_INTERVAL);
    }
    val = val/CALIBARAION_SAMPLE_TIMES;      //calculate the average value
    val = val/RO_CLEAN_AIR_FACTOR;           //divided by RO_CLEAN_AIR_FACTOR yields the Ro according to the chart in the datasheet 
    return val; 
}

/**
 * @brief 
 * 
 * @param raw 
 * @return float 
 */
float mq2::ResistanceCalculation(uint16_t raw)
{
    float adc = (float)(raw * 1.00F);
    return (((float)RL_VALUE*(4095.00F-adc)/adc));
}

/**
 * @brief 
 * 
 * @return float 
 */
float mq2::Read(void)
{
    uint8_t i = 0;
    uint16_t adc = 0;
    float rs = 0;

    for (i=0;i<READ_SAMPLE_TIMES;i++) 
    {
        //take multiple samples
        //adc_get_value(gpio, &adc);
        //adc = adc1_get_raw((adc1_channel_t)ADC1_CHANNEL_0);
        adc = get_value_adc(index);
        rs += ResistanceCalculation(adc);
        delay(READ_SAMPLE_INTERVAL);
    }
    rs = rs/READ_SAMPLE_TIMES;
    return rs; 
}

/**
 * @brief 
 * 
 * @param rs_ro_ratio 
 * @param gas_id 
 * @return float 
 */
float mq2::GetGasPercentage(float rs_ro_ratio, MQ2GasType_t gas_id)
{
    switch(gas_id)
    {
        case GAS_LPG: 
            return GetPercentage(rs_ro_ratio,LPGCurve);
        break;
        case GAS_CO: 
            return GetPercentage(rs_ro_ratio,COCurve);
        break;
        case GAS_SMOKE: 
            return GetPercentage(rs_ro_ratio,SmokeCurve);
        break;
        default: 
            return -1.0;
        break;
    } 
}

/**
 * @brief 
 * 
 * @param rs_ro_ratio 
 * @param pcurve 
 * @return float 
 */
float mq2::GetPercentage(float rs_ro_ratio, float *pcurve)
{
    return (float)( (double) pow(10, ( ( ( log(rs_ro_ratio) - pcurve[1] ) / pcurve[2]) + pcurve[0] ) ) );
}

/**
 * @brief Get the index object
 * 
 * @param gpio 
 * @param id 
 * @return boolean 
 */
boolean mq2::get_index(uint8_t gpio, uint8_t *id)
{
    uint8_t i = 0;
    for(i=0;i<NUMBER_PIN_ADC;i++)
    {
        if(ADC_PIN_MAP[i].gpio == gpio)
        {
            *id = i;
            return true;
        } 
    }
    return false;
}

/**
 * @brief Get the value adc object
 * 
 * @param id 
 * @return uint16_t 
 */
uint16_t mq2::get_value_adc(uint8_t id)
{
    uint16_t ret = 0;

    if(ADC_PIN_MAP[id].channel == Channel_ADC1)
    {
        ret = adc1_get_raw((adc1_channel_t)ADC_PIN_MAP[id].pin_adc);
    }
    else if(ADC_PIN_MAP[id].channel == Channel_ADC2)
    {
        if(adc2_get_raw((adc2_channel_t)ADC_PIN_MAP[id].pin_adc,ADC_WIDTH_12Bit,(int*)&ret) != ESP_OK)
        {
            ret = 0;
        }
    }
    else
    {
        ret = 0;
    }
    return ret;
}