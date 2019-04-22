
#include "nmea_receiver.h"
#include <string.h>
#include "uart_driver.h"
#include "Arduino_HAL.h"

#if 1
static inline uint8_t NMEA_byte_available() {
	return Serial_Available(&Serial_GPS);
}

static inline uint8_t NMEA_read_byte() {

	static uint8_t c=0x00;
	Serial_Read(&Serial_GPS, (char *)&c);
	return c;
}
#endif
static inline void NMEA_flush_received_bytes(struct NMEA_Receiver_struct * ptr)
{
	while(ptr->NMEA_byte_available_func())
	{
		ptr->NMEA_read_byte_func();
	}
}

static inline uint32_t NMEA_Elapsed_time_ms32(struct NMEA_Receiver_struct * ptr,uint32_t * ref) {
	return ptr->NMEA_Get_time_ms32_func() - *ref; //Renseigner la fonction de comparaison de millisecondes
}

static inline uint32_t NMEA_Get_time_ms32() {
	return millis(); //Renseigner la fonction de millis
}

static inline void NMEA_receiver_frame_received(struct NMEA_Receiver_struct * ptr)
{
	ptr->phase_ui8 = 0;

	ptr->flags_nmea_receiver |= NMEA_FLAG_FRAME_READY;
	ptr->flags_nmea_receiver &= ~NMEA_FLAG_AUTORIZE_EXEC; //Stopper le receveur en attendant un reset.
}

static uint8_t AsciiToHex(char h) {

	if ('0' <= h && h <= '9') {
		return h - '0';
	} else if ('a' <= h && h <= 'f') {
		return h - 'a' + 10;
	} else if ('A' <= h && h <= 'F') {
		return h - 'A' + 10;
	} else {
		return 0xF0;
	}
}

//Chaque phase ne traite que l'octet courant

static inline void phase0(struct NMEA_Receiver_struct * ptr) {

	if(ptr->error_flags_ui8) //Si il y a eu une erreur, veiller à vider le buffer de reception uart
	{
		NMEA_flush_received_bytes(ptr);
	}


	ptr->nbr_bytes_rx_ui16 = 0;
	ptr->error_flags_ui8 = 0;

	//Passage à la phase 1
	ptr->phase_ui8 = 1;
}

static inline void phase1(struct NMEA_Receiver_struct * ptr) {

	//On a recu l'octet de debut de trame
	if (ptr->local_byte_rx_ui8 == '$') {

		//On a recu un octet de debut de trame alors reset du buffer et stockage du '$'
		ptr->nbr_bytes_rx_ui16 = 0;
		ptr->buffer_rx_nmea_uc[ptr->nbr_bytes_rx_ui16++] = ptr->local_byte_rx_ui8;

		ptr->checksum_calc_ui8 = 0; //Reset checksum
		ptr->checksum_rx_ui8 = 0; //Reset checksum

		ptr->ref_time_frame_rx_ui32 = ptr->NMEA_Get_time_ms32_func(); //Demarrage timeout trame
		ptr->phase_ui8 = 2;
	}
}

static inline void phase2(struct NMEA_Receiver_struct * ptr) {

	if (ptr->local_byte_rx_ui8 == '*') //Caractere de fin de trame
	{
		ptr->phase3_remaining_bytes_ui8 = 4; //2 char de chksum + CR + LF
		ptr->phase_ui8 = 3; //Fin de reception trame encore 2 octets


	}
	else {
		ptr->checksum_calc_ui8 ^= ptr->local_byte_rx_ui8; //accumulation du checksum




	}
	//Sauvegarde de l'octet dans le buffer de trame
	if (ptr->nbr_bytes_rx_ui16 >= (NMEA_MAX_FRAME_LENGTH - 1)) {

		ptr->nbr_bytes_rx_ui16 = 0; //Eviter overflow buffer
		//Si on overflow pendant la phase 2 ou 3, alors on a une erreur de buffer overflow, on ne peut pas recevoir la trame en entier.
		//Juste resetter et attendre une autre trame
		ptr->error_flags_ui8 |= NMEA_ERROR_BUFFER_OVERFLOW_MSK;

		ptr->phase_ui8 = 4;
	} else {
		ptr->buffer_rx_nmea_uc[ptr->nbr_bytes_rx_ui16++] = ptr->local_byte_rx_ui8; //Ajouter l'octet courant au buffer
	}
}

static inline void phase3(struct NMEA_Receiver_struct * ptr) {

	//Receive the last 4 bytes : 2 chksum char + CR LF
	//When the last char is received trigger the chksum checking
	if (ptr->phase3_remaining_bytes_ui8 > 0) {
		ptr->phase3_remaining_bytes_ui8--;
		//Sauvegarde de l'octet dans le buffer de trame
		if (ptr->nbr_bytes_rx_ui16 >= (NMEA_MAX_FRAME_LENGTH - 1)) {

			ptr->nbr_bytes_rx_ui16 = 0; //Eviter overflow buffer
			//Si on overflow pendant la phase 2 ou 3, alors on a une erreur de buffer overflow, on ne peut pas recevoir la trame en entier.
			//Juste resetter et attendre une autre trame
			ptr->error_flags_ui8 |= NMEA_ERROR_BUFFER_OVERFLOW_MSK;

			ptr->phase_ui8 = 4;
		} else {
			ptr->buffer_rx_nmea_uc[ptr->nbr_bytes_rx_ui16++] = ptr->local_byte_rx_ui8; //Ajouter l'octet courant au buffer
		}

	}

	if(ptr->phase3_remaining_bytes_ui8==0)//Received the 4 bytes of the checksum
	{
		uint8_t msb = ptr->buffer_rx_nmea_uc[ptr->nbr_bytes_rx_ui16 - 4];
		msb = AsciiToHex(msb);
		uint8_t lsb = ptr->buffer_rx_nmea_uc[ptr->nbr_bytes_rx_ui16 - 3];
		lsb = AsciiToHex(lsb);
		//Convert the 2 last bytes to a uint8_t

		if ((msb & 0xF0) || (lsb & 0xF0)) //On a pas recu des octets hexa
		{
			ptr->error_flags_ui8 |= NMEA_ERROR_CHKSUM_MSK;

			ptr->phase_ui8 = 4; //Goto error

		} else if ((msb << 4 | lsb) == ptr->checksum_calc_ui8) //Checksum recu OK ?
		{

			//Reception OK declencher Gerer trame NMEA

			NMEA_receiver_frame_received(ptr);

		} else {
			ptr->error_flags_ui8 |= NMEA_ERROR_CHKSUM_MSK;

			ptr->phase_ui8 = 4; //Goto error
		}

	}
}

static inline void phase4Error(struct NMEA_Receiver_struct * ptr) {

	if(ptr->error_flags_ui8 | NMEA_ERROR_BUFFER_OVERFLOW_MSK)
		ptr->nb_error_buffer_overflow++;

	if(ptr->error_flags_ui8 | NMEA_ERROR_CHKSUM_MSK)
		ptr->nb_error_chksum++;

	if(ptr->error_flags_ui8 | NMEA_ERROR_FRAME_TIMEOUT_MSK | NMEA_ERROR_FRAME_TIMEOUT_RXEMTPY_MSK)
		ptr->nb_error_timeout++;

	//An error occured, reset everything and retry
	NMEA_receiver_reset(ptr);
}

void NMEA_receiver_init(struct NMEA_Receiver_struct * ptr) {
	//Init de la structure
	memset(ptr,0,sizeof(struct NMEA_Receiver_struct));
	ptr->NMEA_byte_available_func= NMEA_byte_available;
	ptr->NMEA_read_byte_func= NMEA_read_byte;
	ptr->NMEA_Get_time_ms32_func= NMEA_Get_time_ms32;
	ptr->frame_timeout_value_ui32=100;
	ptr->interframe_timeout_value_ui32=2000;
	//Demarrage du driver
	NMEA_receiver_reset(ptr);
}

/**
 * Appeler cette procédure lorsqu'une trame nmea a été parsé.
 * @param ptr
 */
void NMEA_receiver_reset(struct NMEA_Receiver_struct * ptr) {
	ptr->phase_ui8 = 0;
	ptr->error_flags_ui8=0;
	ptr->flags_nmea_receiver &= ~NMEA_FLAG_FRAME_READY;
	ptr->flags_nmea_receiver |= NMEA_FLAG_AUTORIZE_EXEC;
}

void NMEA_receiver_task(struct NMEA_Receiver_struct * ptr) {

	//Hot init du driver, avant reception trame
	if (ptr->phase_ui8 == 0) {
		phase0(ptr);
	}

	//Peut-t-on recevoir une trame NMEA ?
	if (ptr->flags_nmea_receiver & NMEA_FLAG_AUTORIZE_EXEC) {

		if (ptr->phase_ui8 & 0x03) { //Si on attend un octet p1 p2 p3


			//Si un octet est dispo en fifo
			while (ptr->NMEA_byte_available_func()) {

				//Recup de l'octet
				ptr->local_byte_rx_ui8 = ptr->NMEA_read_byte_func();

				//printf("%c",ptr->local_byte_rx_ui8);


				//Executer la phase de reception octet/Traiter l'octet courant
				switch (ptr->phase_ui8) {
				case 1:
					phase1(ptr); //Reception 1er octet
					break;
				case 2:
					phase2(ptr); //Reception jusqu'à '*' + chksum + CR + LF
					break;
				case 3:
					phase3(ptr); //Reception du checksum apres '*'
					break;
				}
				//Temps de reception de trame trop long, mais il reste des octets à traiter
				if (ptr->phase_ui8 > 1 && NMEA_Elapsed_time_ms32(ptr,&(ptr->ref_time_frame_rx_ui32)) >= ptr->frame_timeout_value_ui32) {
					ptr->error_flags_ui8 |= NMEA_ERROR_FRAME_TIMEOUT_MSK; //Erreur timeout
					//La trame a mis trop de temps à etre recue

					ptr->phase_ui8=4;
					break;
				}
			}

			//Checker le timeout de fin de trame si on est en reception octets de trame et fin de trame
			if (ptr->phase_ui8 > 1 && (NMEA_Elapsed_time_ms32(ptr,&(ptr->ref_time_frame_rx_ui32)) >= ptr->frame_timeout_value_ui32)) {
				//Trame non reçue dans les temps
				//On ne reçoit plus d'octets, on a pas recu la fin de trame dans les temps.
				ptr->error_flags_ui8 |= NMEA_ERROR_FRAME_TIMEOUT_RXEMTPY_MSK;

				ptr->phase_ui8 = 4; //Goto error
			}

		}

		if(ptr->phase_ui8 == 4)
		{
			phase4Error(ptr);
		}
	}

	//Si le temps écoulé depuis la derniere frame est supérieur au timeout
	if(NMEA_Elapsed_time_ms32(ptr, &ptr->ref_time_frame_rx_ui32) >= ptr->interframe_timeout_value_ui32)
	{
		ptr->flag_is_gps_active=0;
	}
	else
	{
		ptr->flag_is_gps_active=1; //le temps est inferieur donc le gps est actif
	}

}

uint8_t NMEA_receiver_frame_ready(struct NMEA_Receiver_struct * ptr)
{
	return ptr->flags_nmea_receiver & NMEA_FLAG_FRAME_READY ? 1:0;
}
