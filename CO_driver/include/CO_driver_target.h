/*
 * Device and application specific definitions for CANopenNode.
 *
 * @file        CO_driver_target.h
 * @author      Janez Paternoster
 * @copyright   2020 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CO_DRIVER_TARGET
#define CO_DRIVER_TARGET

/* This file contains device and application specific definitions.
 * It is included from CO_driver.h, which contains documentation
 * for definitions below. */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "driver/twai.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#ifdef CO_DRIVER_CUSTOM
#include "CO_driver_custom.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* Stack configuration override from CO_driver.h.
 * For more information see file CO_config.h. 
 https://github.com/CANopenNode/CANopenPIC/blob/21fa73185bcf5392738b5239958bb67c36fb4e89/PIC24_dsPIC33/CO_driver_target.h*/
#ifndef CO_CONFIG_NMT
#define CO_CONFIG_NMT CO_CONFIG_NMT_MASTER
#endif

#ifndef CO_CONFIG_HB_CONS
#ifdef CONFIG_CANOPEN_MONITOR_HB
#define CO_CONFIG_HB_CONS (CO_CONFIG_HB_CONS_ENABLE |          \
                           CO_CONFIG_HB_CONS_CALLBACK_CHANGE | \
                           CO_CONFIG_FLAG_TIMERNEXT)
#endif
#endif

#ifndef CO_CONFIG_SDO_CLI
#define CO_CONFIG_SDO_CLI (CO_CONFIG_SDO_CLI_ENABLE)
#endif

#ifndef CO_CONFIG_FIFO
#define CO_CONFIG_FIFO (CO_CONFIG_FIFO_ENABLE)
#endif

// Custom
#define CO_CONFIG_STORAGE (0)
#define CO_CONFIG_LSS (0)
#define CO_CONFIG_LEDS (0)
#define CO_CONFIG_TIME (0)

/* Basic definitions. If big endian, CO_SWAP_xx macros must swap bytes. */
#define CO_LITTLE_ENDIAN
#define CO_SWAP_16(x) x
#define CO_SWAP_32(x) x
#define CO_SWAP_64(x) x
    /* NULL is defined in stddef.h */
    /* true and false are defined in stdbool.h */
    /* int8_t to uint64_t are defined in stdint.h */
    typedef unsigned char bool_t;
    typedef float float32_t;
    typedef double float64_t;
    typedef char char_t;
    typedef unsigned char oChar_t;
    typedef unsigned char domain_t;

/* Access to received CAN message */
#define CO_CANrxMsg_readIdent(msg) ((uint16_t)((twai_message_t *)msg)->identifier)
#define CO_CANrxMsg_readDLC(msg) ((uint8_t)((twai_message_t *)msg)->data_length_code)
#define CO_CANrxMsg_readData(msg) ((uint8_t *)((twai_message_t *)msg)->data)

    /* Received message object */
    typedef struct
    {
        uint16_t ident;
        uint16_t mask;
        void *object;
        void (*CANrx_callback)(void *object, void *message);
    } CO_CANrx_t;

    /* Transmit message object */
    typedef struct
    {
        uint32_t ident;
        uint8_t DLC;
        uint8_t data[8];
        volatile bool_t bufferFull;
        volatile bool_t syncFlag;
    } CO_CANtx_t;

    /* CAN module object */
    typedef struct
    {
        void *CANptr;
        CO_CANrx_t *rxArray;
        uint16_t rxSize;
        CO_CANtx_t *txArray;
        uint16_t txSize;
        uint16_t CANerrorStatus;
        volatile bool_t CANnormal;
        volatile bool_t useCANrxFilters;
        volatile bool_t bufferInhibitFlag;
        volatile bool_t firstCANtxMessage;
        volatile uint16_t CANtxCount;
        uint32_t errOld;
    } CO_CANmodule_t;

    /* Data storage object for one entry */
    typedef struct
    {
        void *addr;
        size_t len;
        uint8_t subIndexOD;
        uint8_t attr;
        /* Additional variables (target specific) */
        void *addrNV;
    } CO_storage_entry_t;

/* (un)lock critical section in CO_CANsend() */
#define CO_LOCK_CAN_SEND(CAN_MODULE)
#define CO_UNLOCK_CAN_SEND(CAN_MODULE)

/* (un)lock critical section in CO_errorReport() or CO_errorReset() */
#define CO_LOCK_EMCY(CAN_MODULE)
#define CO_UNLOCK_EMCY(CAN_MODULE)

/* (un)lock critical section when accessing Object Dictionary */
#define CO_LOCK_OD(CAN_MODULE)
#define CO_UNLOCK_OD(CAN_MODULE)

/* Synchronization between CAN receive and message processing threads. */
#define CO_MemoryBarrier()
#define CO_FLAG_READ(rxNew) ((rxNew) != NULL)
#define CO_FLAG_SET(rxNew)  \
    {                       \
        CO_MemoryBarrier(); \
        rxNew = (void *)1L; \
    }
#define CO_FLAG_CLEAR(rxNew) \
    {                        \
        CO_MemoryBarrier();  \
        rxNew = NULL;        \
    }

    void CANreceive(CO_CANmodule_t *CANmodule);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CO_DRIVER_TARGET */
