#include "gd32f10x.h"

#define I2C_10BIT_ADDRESS      0

#define I2C0_OWN_ADDRESS7      0x82
#define I2C0_SLAVE_ADDRESS7    0x72
#define I2C0_OWN_ADDRESS10     0x0322
#define I2C1                          (I2C_BASE + 0x00000400U)   /*!< I2C1 base address */
I2C_FLAG_ADDSEND = I2C_REGIDX_BIT(I2C_STAT0_REG_OFFSET, 1U),               /*!< address is sent in master mode or received and matches in slave mode */
 #define I2C_REGIDX_BIT(regidx, bitpos)  (((uint32_t)(regidx) << 6) | (uint32_t)(bitpos))

uint8_t i2c_transmitter[16];

void rcu_config(void);
void gpio_config(void);
void i2c_config(void);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    int i;
    
    /* RCU config */
    rcu_config();
    /* GPIO config */
    gpio_config();
    /* I2C config */
    i2c_config();

    for(i=0; i<16; i++){
        i2c_transmitter[i] = i+0x80;
    }
    
#if I2C_10BIT_ADDRESS
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
#else
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
#endif
    /* wait until the transmission data register is empty */
    while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));

    for(i=0;i<16;i++){
        /* send a data byte */
        i2c_data_transmit(I2C0, i2c_transmitter[i]);
        /* wait until the transmission data register is empty */
        while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
    }
    /* the master doesn't acknowledge for the last byte */
    while(!i2c_flag_get(I2C0, I2C_FLAG_AERR));
    /* clear the bit of AERR */
    i2c_flag_clear(I2C0, I2C_FLAG_AERR);

    while(1){
    }
}

/*!
    \brief      enable the peripheral clock
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable GPIOB clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    /* enable I2C0 clock */
    rcu_periph_clock_enable(RCU_I2C0);
}

/*!
    \brief      cofigure the GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
*/
void gpio_config(void)
{
    /* connect PB6 to I2C0_SCL */
    /* connect PB7 to I2C0_SDA */
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
}

/*!
    \brief      cofigure the I2C0 and I2C1 interfaces
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_config(void)
{
    /* I2C clock configure */
    i2c_clock_config(I2C0, 400000, I2C_DTCY_2);
    /* I2C address configure */
#if I2C_10BIT_ADDRESS
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_10BITS, I2C0_OWN_ADDRESS10);
#else
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, I2C0_OWN_ADDRESS7);
#endif
    /* enable I2C0 */
    i2c_enable(I2C0);
    /* enable acknowledge */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
}

/*!
    \brief      check I2C flag is set or not
    \param[in]  i2c_periph: I2Cx(x=0,1)
    \param[in]  flag: I2C flags, refer to i2c_flag_enum
                only one parameter can be selected which is shown as below:
      \arg        I2C_FLAG_SBSEND: start condition send out 
      \arg        I2C_FLAG_ADDSEND: address is sent in master mode or received and matches in slave mode
      \arg        I2C_FLAG_BTC: byte transmission finishes
      \arg        I2C_FLAG_ADD10SEND: header of 10-bit address is sent in master mode
      \arg        I2C_FLAG_STPDET: stop condition detected in slave mode
      \arg        I2C_FLAG_RBNE: I2C_DATA is not Empty during receiving
      \arg        I2C_FLAG_TBE: I2C_DATA is empty during transmitting
      \arg        I2C_FLAG_BERR: a bus error occurs indication a unexpected start or stop condition on I2C bus
      \arg        I2C_FLAG_LOSTARB: arbitration lost in master mode
      \arg        I2C_FLAG_AERR: acknowledge error
      \arg        I2C_FLAG_OUERR: overrun or underrun situation occurs in slave mode
      \arg        I2C_FLAG_PECERR: PEC error when receiving data
      \arg        I2C_FLAG_SMBTO: timeout signal in SMBus mode
      \arg        I2C_FLAG_SMBALT: SMBus alert status
      \arg        I2C_FLAG_MASTER: a flag indicating whether I2C block is in master or slave mode
      \arg        I2C_FLAG_I2CBSY: busy flag
      \arg        I2C_FLAG_TRS: whether the I2C is a transmitter or a receiver
      \arg        I2C_FLAG_RXGC: general call address (00h) received
      \arg        I2C_FLAG_DEFSMB: default address of SMBus device
      \arg        I2C_FLAG_HSTSMB: SMBus host header detected in slave mode
      \arg        I2C_FLAG_DUMOD: dual flag in slave mode indicating which address is matched in dual-address mode
    \param[out] none
    \retval     FlagStatus: SET or RESET
*/
FlagStatus i2c_flag_get(uint32_t i2c_periph, i2c_flag_enum flag)
{
    if(RESET != (I2C_REG_VAL(i2c_periph, flag) & BIT(I2C_BIT_POS(flag)))){
        return SET;
    }else{
        return RESET;
    }
}
