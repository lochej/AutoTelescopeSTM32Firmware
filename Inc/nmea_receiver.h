/* 
 * File:   nmea_receiver.h
 * Author: JLH
 *
 * Created on October 29, 2018, 8:25 PM
 */

#ifndef NMEA_RECEIVER_H
#define	NMEA_RECEIVER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

#define NMEA_MAX_FRAME_LENGTH (128)
    //Frame timeout error bit
#define NMEA_ERROR_FRAME_TIMEOUT_MSK (0x01<<0)
    //Frame timeout error, frame receive too slow
#define NMEA_ERROR_FRAME_TIMEOUT_RXEMTPY_MSK (0x01<<1)
    //Byte receiving tiemout error bit
#define NMEA_BYTES_TOE_MSK (0x01<<2)
    //Received frame chksum not valid
#define NMEA_ERROR_CHKSUM_MSK (0x01<<3)
    
#define NMEA_ERROR_BUFFER_OVERFLOW_MSK (0x01<<4)
    
#define NMEA_FLAG_AUTORIZE_EXEC (0x01<<0)
    
#define NMEA_FLAG_FRAME_READY (0x01<<1)
    
    typedef uint8_t (*NMEA_read_byte_func)(void);
    typedef uint8_t (*NMEA_byte_available_func)(void);
    typedef uint32_t (*NMEA_Get_time_ms32_func)(void);

    struct NMEA_Receiver_struct
    {
        uint8_t buffer_rx_nmea_uc[NMEA_MAX_FRAME_LENGTH];
        uint8_t local_byte_rx_ui8;
        uint16_t nbr_bytes_rx_ui16;
        
        uint8_t phase3_remaining_bytes_ui8;
        uint8_t checksum_rx_ui8;
        uint8_t checksum_calc_ui8;
        
        
        uint32_t ref_time_frame_rx_ui32;
        //uint32_t ref_time_byte_rx_ui32;
        uint32_t ref_time_interframe_ui32;
        

        // valeur des timeouts programmes
        uint32_t frame_timeout_value_ui32;
        //GPS considere comme inactif si 2 frames consécutives
        //sont reçu avec un intervalle de temps supérieur a interframe_timeout_value
        uint32_t interframe_timeout_value_ui32;

        //Grafcet reception buffer nmea
        uint8_t error_flags_ui8; //Flags d'erreurs
        uint8_t phase_ui8; //phase à executer
        uint8_t flags_nmea_receiver;
        uint8_t flag_is_gps_active;
        uint8_t nb_error_timeout;
        uint8_t nb_error_chksum;
        uint8_t nb_error_buffer_overflow;
        


        uint8_t (*NMEA_read_byte_func)(void);
        uint8_t (*NMEA_byte_available_func)(void);
        uint32_t (*NMEA_Get_time_ms32_func)(void);
                
    };

    void NMEA_receiver_init(struct NMEA_Receiver_struct * ptr);
    void NMEA_receiver_task(struct NMEA_Receiver_struct * ptr);
    void NMEA_receiver_reset(struct NMEA_Receiver_struct * ptr);
    uint8_t NMEA_receiver_frame_ready(struct NMEA_Receiver_struct * ptr);

#ifdef	__cplusplus
}
#endif

#endif	/* NMEA_RECEIVER_H */

