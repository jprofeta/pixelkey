/* generated vector header file - do not edit */
#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H
#ifdef __cplusplus
        extern "C" {
        #endif
/* Number of interrupts allocated */
#ifndef VECTOR_DATA_IRQ_COUNT
#define VECTOR_DATA_IRQ_COUNT    (11)
#endif
/* ISR prototypes */
void usbfs_interrupt_handler(void);
void usbfs_resume_handler(void);
void usbfs_d0fifo_handler(void);
void usbfs_d1fifo_handler(void);
void dmac_int_isr(void);
void dmac2_repeat_isr(void);
void rtc_alarm_periodic_isr(void);
void rtc_carry_isr(void);
void gpt_counter_overflow_isr(void);
void dtc_complete_isr(void);

/* Vector table allocations */
#define VECTOR_NUMBER_USBFS_INT ((IRQn_Type) 0) /* USBFS INT (USBFS interrupt) */
#define USBFS_INT_IRQn          ((IRQn_Type) 0) /* USBFS INT (USBFS interrupt) */
#define VECTOR_NUMBER_USBFS_RESUME ((IRQn_Type) 1) /* USBFS RESUME (USBFS resume interrupt) */
#define USBFS_RESUME_IRQn          ((IRQn_Type) 1) /* USBFS RESUME (USBFS resume interrupt) */
#define VECTOR_NUMBER_USBFS_FIFO_0 ((IRQn_Type) 2) /* USBFS FIFO 0 (DMA transfer request 0) */
#define USBFS_FIFO_0_IRQn          ((IRQn_Type) 2) /* USBFS FIFO 0 (DMA transfer request 0) */
#define VECTOR_NUMBER_USBFS_FIFO_1 ((IRQn_Type) 3) /* USBFS FIFO 1 (DMA transfer request 1) */
#define USBFS_FIFO_1_IRQn          ((IRQn_Type) 3) /* USBFS FIFO 1 (DMA transfer request 1) */
#define VECTOR_NUMBER_DMAC0_INT ((IRQn_Type) 4) /* DMAC0 INT (DMAC transfer end 0) */
#define DMAC0_INT_IRQn          ((IRQn_Type) 4) /* DMAC0 INT (DMAC transfer end 0) */
#define VECTOR_NUMBER_DMAC1_INT ((IRQn_Type) 5) /* DMAC1 INT (DMAC transfer end 1) */
#define DMAC1_INT_IRQn          ((IRQn_Type) 5) /* DMAC1 INT (DMAC transfer end 1) */
#define VECTOR_NUMBER_DMAC2_INT ((IRQn_Type) 6) /* DMAC2 INT (DMAC transfer end 2) */
#define DMAC2_INT_IRQn          ((IRQn_Type) 6) /* DMAC2 INT (DMAC transfer end 2) */
#define VECTOR_NUMBER_RTC_PERIOD ((IRQn_Type) 7) /* RTC PERIOD (Periodic interrupt) */
#define RTC_PERIOD_IRQn          ((IRQn_Type) 7) /* RTC PERIOD (Periodic interrupt) */
#define VECTOR_NUMBER_RTC_CARRY ((IRQn_Type) 8) /* RTC CARRY (Carry interrupt) */
#define RTC_CARRY_IRQn          ((IRQn_Type) 8) /* RTC CARRY (Carry interrupt) */
#define VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW ((IRQn_Type) 9) /* GPT0 COUNTER OVERFLOW (Overflow) */
#define GPT0_COUNTER_OVERFLOW_IRQn          ((IRQn_Type) 9) /* GPT0 COUNTER OVERFLOW (Overflow) */
#define VECTOR_NUMBER_DTC_COMPLETE ((IRQn_Type) 10) /* DTC COMPLETE (DTC last transfer) */
#define DTC_COMPLETE_IRQn          ((IRQn_Type) 10) /* DTC COMPLETE (DTC last transfer) */
#ifdef __cplusplus
        }
        #endif
#endif /* VECTOR_DATA_H */
