/**
 * @file mq2.h
 * @author Evandro Teixeira
 * @brief 
 * @version 0.1
 * @date 31-08-2020
 * 
 * @copyright Copyright (c) 2020
 * @note based on the algorithm https://os.mbed.com/users/azazeal88/code/MQ2/
 */
#include <Arduino.h>

/**
 * @brief 
 * 
 */
typedef enum  
{
    GAS_LPG = 0,
    GAS_CO = 1,
    GAS_SMOKE = 2
} MQ2GasType_t;

/**
 * @brief 
 * 
 */
class mq2
{
    private:
    /**
     * @brief 
     * 
     */
    uint8_t index;
    /**
     * @brief 
     * 
     */
    float R0;
    /**
     * @brief 
     * 
     * @return float 
     */
    float Read();
    /**
     * @brief 
     * 
     * @return float 
     */
    float Calibration(void);
    /**
     * @brief 
     * 
     * @param raw 
     * @return float 
     */
    float ResistanceCalculation(uint16_t raw);
    /**
     * @brief Get the Gas Percentage object
     * 
     * @param rs_ro_ratio 
     * @param gas_id 
     * @return float 
     */
    float GetGasPercentage(float rs_ro_ratio, MQ2GasType_t gas_id);
    /**
     * @brief Get the Percentage object
     * 
     * @param rs_ro_ratio 
     * @param pcurve 
     * @return float 
     */
    float GetPercentage(float rs_ro_ratio, float *pcurve);
    /**
     * @brief Get the index object
     * 
     * @param gpio 
     * @param id 
     * @return boolean 
     */
    boolean get_index(uint8_t gpio, uint8_t *id);
    /**
     * @brief Get the value adc object
     * 
     * @param id 
     * @return uint16_t 
     */
    uint16_t get_value_adc(uint8_t id);

    public:
    /**
     * @brief Construct a new mq2 object
     * 
     * @param io 
     */
    mq2(uint8_t io);
    /**
     * @brief Destroy the mq2 object
     * 
     */
    ~mq2();
    /**
     * @brief 
     * 
     */
    void begin(void);
    /**
     * @brief 
     * 
     * @param type 
     * @return float 
     */
    float get(MQ2GasType_t type);
};
