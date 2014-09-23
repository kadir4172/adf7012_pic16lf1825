/* 
 * File:   config.h
 * Author: Kadir
 *
 * Created on September 21, 2014, 3:31 PM
 */

#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif


#define VCXO_FREQ 20000000L    //VCO 20Mhz
#define ADF7012_CRYSTAL_DIVIDER 15 //PFD = 20/15 Mhz
#define RADIO_FREQUENCY   144390000UL //Beacon center frequency






#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

