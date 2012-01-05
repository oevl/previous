void SCSI_CSR0_Read(void); 
void SCSI_CSR0_Write(void);
void SCSI_CSR1_Read(void);
void SCSI_CSR1_Write(void);

void SCSI_TransCountL_Read(void); 
void SCSI_TransCountL_Write(void); 
void SCSI_TransCountH_Read(void);
void SCSI_TransCountH_Write(void); 
void SCSI_FIFO_Read(void);
void SCSI_FIFO_Write(void); 
void SCSI_Command_Read(void); 
void SCSI_Command_Write(void); 
void SCSI_Status_Read(void);
void SCSI_SelectBusID_Write(void); 
void SCSI_IntStatus_Read(void);
void SCSI_SelectTimeout_Write(void); 
void SCSI_SeqStep_Read(void);
void SCSI_SyncPeriod_Write(void); 
void SCSI_FIFOflags_Read(void);
void SCSI_SyncOffset_Write(void); 
void SCSI_Configuration_Read(void); 
void SCSI_Configuration_Write(void); 
void SCSI_ClockConv_Write(void);
void SCSI_Test_Write(void);


void handle_satn(void);
void handle_ti(void);
void esp_raise_irq(void);
void esp_lower_irq(void);
Uint32 get_cmd(void);
void do_cmd(void);
void do_busid_cmd(Uint8);
void esp_do_dma(void);
void esp_dma_done(void);
void esp_transfer_data(void);