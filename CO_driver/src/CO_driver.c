/*
 * CAN module object for generic microcontroller.
 *
 * This file is a template for other microcontrollers.
 *
 * @file        CO_driver.c
 * @ingroup     CO_driver
 * @author      Janez Paternoster
 * @copyright   2004 - 2020 Janez Paternoster
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

#include "301/CO_driver.h"

#include "esp_log.h"

#define CO_DRIVER_TAG "co-driver"

static const twai_general_config_t g_config =
    TWAI_GENERAL_CONFIG_DEFAULT(CONFIG_CAN_TX_GPIO, CONFIG_CAN_RX_GPIO, TWAI_MODE_NORMAL);

#ifdef CONFIG_CANOPEN_BITRATE_25KBITS
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_25KBITS();
#endif
#ifdef CONFIG_CANOPEN_BITRATE_50KBITS
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_50KBITS();
#endif
#ifdef CONFIG_CANOPEN_BITRATE_100KBITS
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_100KBITS();
#endif
#ifdef CONFIG_CANOPEN_BITRATE_125KBITS
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();
#endif
#ifdef CONFIG_CANOPEN_BITRATE_250KBITS
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
#endif
#ifdef CONFIG_CANOPEN_BITRATE_500KBITS
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
#endif
#ifdef CONFIG_CANOPEN_BITRATE_800KBITS
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_800KBITS();
#endif
#ifdef CONFIG_CANOPEN_BITRATE_1MBITS
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
#endif

static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

/******************************************************************************/
void CO_CANsetConfigurationMode(void *CANptr)
{
  /* Put CAN module in configuration mode */
  ESP_LOGI(CO_DRIVER_TAG, "CO_CANsetConfigurationMode");
}

/******************************************************************************/
void CO_CANsetNormalMode(CO_CANmodule_t *CANmodule)
{
  /* Put CAN module in normal mode */

  CANmodule->CANnormal = true;

  if (twai_start() == ESP_OK)
  {
    ESP_LOGI(CO_DRIVER_TAG, "Driver started\n");
  }
  else
  {
    ESP_LOGI(CO_DRIVER_TAG, "Failed to start driver\n");
    // ESP_LOGI(CO_DRIVER_TAG, "CO_CANsetNormalMode");
  }
}
/******************************************************************************/
CO_ReturnError_t CO_CANmodule_init(
    CO_CANmodule_t *CANmodule,
    void *CANptr,
    CO_CANrx_t rxArray[],
    uint16_t rxSize,
    CO_CANtx_t txArray[],
    uint16_t txSize,
    uint16_t CANbitRate)
{
  uint16_t i;

  /* verify arguments */
  if (CANmodule == NULL || rxArray == NULL || txArray == NULL)
  {
    return CO_ERROR_ILLEGAL_ARGUMENT;
  }

  /* Configure object variables */
  CANmodule->CANptr = CANptr;
  CANmodule->rxArray = rxArray;
  CANmodule->rxSize = rxSize;
  CANmodule->txArray = txArray;
  CANmodule->txSize = txSize;
  CANmodule->CANerrorStatus = 0;
  CANmodule->CANnormal = false;
  CANmodule->useCANrxFilters = false; // TODO: (rxSize <= 32U) ? true : false; /* microcontroller dependent */
  CANmodule->bufferInhibitFlag = false;
  CANmodule->firstCANtxMessage = true;
  CANmodule->CANtxCount = 0U;
  CANmodule->errOld = 0U;

  for (i = 0U; i < rxSize; i++)
  {
    rxArray[i].ident = 0U;
    rxArray[i].mask = 0xFFFFU;
    rxArray[i].object = NULL;
    rxArray[i].CANrx_callback = NULL;
  }
  for (i = 0U; i < txSize; i++)
  {
    txArray[i].bufferFull = false;
  }

  /* Configure CAN module registers */

  /* Configure CAN timing */

  /* Configure CAN module hardware filters */
  if (CANmodule->useCANrxFilters)
  {
    /* CAN module filters are used, they will be configured with */
    /* CO_CANrxBufferInit() functions, called by separate CANopen */
    /* init functions. */
    /* Configure all masks so, that received message must match filter */
  }
  else
  {
    /* CAN module filters are not used, all messages with standard 11-bit */
    /* identifier will be received */
    /* Configure mask 0 so, that all messages with standard identifier are accepted */
  }

  /* configure CAN interrupt registers */

  ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));

  ESP_LOGI(CO_DRIVER_TAG, "CO_CANmodule_init (Driver installed)");
  return CO_ERROR_NO;
}

/******************************************************************************/
void CO_CANmodule_disable(CO_CANmodule_t *CANmodule)
{
  /* turn off the module */
  ESP_ERROR_CHECK(twai_stop());
  //ESP_ERROR_CHECK(twai_driver_uninstall());

  ESP_LOGI(CO_DRIVER_TAG, "CO_CANmodule_disable (twai_stop)");
}

/******************************************************************************/
CO_ReturnError_t CO_CANrxBufferInit(
    CO_CANmodule_t *CANmodule,
    uint16_t index,
    uint16_t ident,
    uint16_t mask,
    bool_t rtr,
    void *object,
    void (*CANrx_callback)(void *object, void *message))
{
  CO_ReturnError_t ret = CO_ERROR_NO;

  if ((CANmodule != NULL) && (object != NULL) && (CANrx_callback != NULL) && (index < CANmodule->rxSize))
  {
    /* buffer, which will be configured */
    CO_CANrx_t *buffer = &CANmodule->rxArray[index];

    /* Configure object variables */
    buffer->object = object;
    buffer->CANrx_callback = CANrx_callback;

    /* CAN identifier and CAN mask, bit aligned with CAN module. Different on different microcontrollers. */
    buffer->ident = ident & 0x07FFU;
    if (rtr)
    {
      buffer->ident |= 0x0800U;
    }
    buffer->mask = (mask & 0x07FFU) | 0x0800U;

    /* Set CAN hardware module filter and mask. */
    if (CANmodule->useCANrxFilters)
    {
    }

    ESP_LOGI(CO_DRIVER_TAG, "Setup buffer rx[%d] ident: %d mask %d", index, ident, mask);
  }
  else
  {
    ret = CO_ERROR_ILLEGAL_ARGUMENT;
  }

  return ret;
}

/******************************************************************************/
CO_CANtx_t *CO_CANtxBufferInit(
    CO_CANmodule_t *CANmodule,
    uint16_t index,
    uint16_t ident,
    bool_t rtr,
    uint8_t noOfBytes,
    bool_t syncFlag)
{
  CO_CANtx_t *buffer = NULL;

  if ((CANmodule != NULL) && (index < CANmodule->txSize))
  {
    /* get specific buffer */
    buffer = &CANmodule->txArray[index];

    /* CAN identifier, DLC and rtr, bit aligned with CAN module transmit buffer.
         * Microcontroller specific. */
    //buffer->ident = ((uint32_t)ident & 0x07FFU) | ((uint32_t)(((uint32_t)noOfBytes & 0xFU) << 12U)) | ((uint32_t)(rtr ? 0x8000U : 0U));
    buffer->ident = ident & 0x07FFU;
    buffer->DLC = noOfBytes & 0xFU;

    buffer->bufferFull = false;
    buffer->syncFlag = syncFlag;
  }

  ESP_LOGI(CO_DRIVER_TAG, "Setup buffer tx[%d] ident: %d bytes %d", index, ident, noOfBytes);

  return buffer;
}

/******************************************************************************/
CO_ReturnError_t CO_CANsend(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer)
{
  CO_ReturnError_t err = CO_ERROR_NO;

  /* Verify overflow */
  if (buffer->bufferFull)
  {
    if (!CANmodule->firstCANtxMessage)
    {
      /* don't set error, if bootup message is still on buffers */
      CANmodule->CANerrorStatus |= CO_CAN_ERRTX_OVERFLOW;
    }
    err = CO_ERROR_TX_OVERFLOW;
  }

  CO_LOCK_CAN_SEND();
  /* if CAN TX buffer is free, copy message to it */

  twai_message_t msg;
  msg.identifier = buffer->ident;
  msg.data_length_code = buffer->DLC;
  msg.flags = TWAI_MSG_FLAG_NONE;
  // TODO: Use memcpy
  for (uint8_t i = 0; i < 8; i++)
    msg.data[i] = buffer->data[i];

  esp_err_t ret = twai_transmit(&msg, pdMS_TO_TICKS(10));

  if ((ret == ESP_OK) && CANmodule->CANtxCount == 0)
  {
    CANmodule->bufferInhibitFlag = buffer->syncFlag;
    /* copy message and txRequest */

    // ESP_LOGI(CO_DRIVER_TAG, "twai_transmit ident: 0x%03X, DLC: %d 0x[%02X %02X %02X %02X %02X %02X %02X %02X] flags: 0x%08X",
    //          msg.identifier, msg.data_length_code, msg.data[0], msg.data[1], msg.data[2], msg.data[3], msg.data[4],
    //          msg.data[5], msg.data[6], msg.data[7], msg.flags);
  }

  /* if no buffer is free, message will be sent by interrupt */
  else
  {
    ESP_LOGI(CO_DRIVER_TAG, "Tx fail! %d", (int) buffer->ident);
    buffer->bufferFull = true;
    CANmodule->CANtxCount++;
  }

  CO_UNLOCK_CAN_SEND();

  return err;
}

/******************************************************************************/
void CO_CANclearPendingSyncPDOs(CO_CANmodule_t *CANmodule)
{
  ESP_LOGI(CO_DRIVER_TAG, "CO_CANclearPendingSyncPDOs");

  uint32_t tpdoDeleted = 0U;

  CO_LOCK_CAN_SEND();
  /* Abort message from CAN module, if there is synchronous TPDO.
     * Take special care with this functionality. */
  if (/*messageIsOnCanBuffer && */ CANmodule->bufferInhibitFlag)
  {
    /* clear TXREQ */
    twai_clear_transmit_queue();

    CANmodule->bufferInhibitFlag = false;
    tpdoDeleted = 1U;
  }
  /* delete also pending synchronous TPDOs in TX buffers */
  if (CANmodule->CANtxCount != 0U)
  {
    uint16_t i;
    CO_CANtx_t *buffer = &CANmodule->txArray[0];
    for (i = CANmodule->txSize; i > 0U; i--)
    {
      if (buffer->bufferFull)
      {
        if (buffer->syncFlag)
        {
          buffer->bufferFull = false;
          CANmodule->CANtxCount--;
          tpdoDeleted = 2U;
        }
      }
      buffer++;
    }
  }
  CO_UNLOCK_CAN_SEND();

  if (tpdoDeleted != 0U)
  {
    CANmodule->CANerrorStatus |= CO_CAN_ERRTX_PDO_LATE;
  }
}

/******************************************************************************/
/* Get error counters from the module. If necessary, function may use
    * different way to determine errors. */
static uint16_t rxErrors = 0, txErrors = 0, overflow = 0;

void CO_CANmodule_process(CO_CANmodule_t *CANmodule)
{
  // this one is called from main continuously
  uint32_t err;

  err = ((uint32_t)txErrors << 16) | ((uint32_t)rxErrors << 8) | overflow;

  if (CANmodule->errOld != err)
  {
    uint16_t status = CANmodule->CANerrorStatus;

    CANmodule->errOld = err;

    if (txErrors >= 256U)
    {
      /* bus off */
      status |= CO_CAN_ERRTX_BUS_OFF;
    }
    else
    {
      /* recalculate CANerrorStatus, first clear some flags */
      status &= 0xFFFF ^ (CO_CAN_ERRTX_BUS_OFF |
                          CO_CAN_ERRRX_WARNING | CO_CAN_ERRRX_PASSIVE |
                          CO_CAN_ERRTX_WARNING | CO_CAN_ERRTX_PASSIVE);

      /* rx bus warning or passive */
      if (rxErrors >= 128)
      {
        status |= CO_CAN_ERRRX_WARNING | CO_CAN_ERRRX_PASSIVE;
      }
      else if (rxErrors >= 96)
      {
        status |= CO_CAN_ERRRX_WARNING;
      }

      /* tx bus warning or passive */
      if (txErrors >= 128)
      {
        status |= CO_CAN_ERRTX_WARNING | CO_CAN_ERRTX_PASSIVE;
      }
      else if (rxErrors >= 96)
      {
        status |= CO_CAN_ERRTX_WARNING;
      }

      /* if not tx passive clear also overflow */
      if ((status & CO_CAN_ERRTX_PASSIVE) == 0)
      {
        status &= 0xFFFF ^ CO_CAN_ERRTX_OVERFLOW;
      }
    }

    if (overflow != 0)
    {
      /* CAN RX bus overflow */
      status |= CO_CAN_ERRRX_OVERFLOW;
    }

    CANmodule->CANerrorStatus = status;
  }
}

/******************************************************************************/

void CANreceive(CO_CANmodule_t *CANmodule)
{
  twai_message_t rcvMsg;      /* received message in CAN module */
  uint16_t index;            /* index of received message */
  uint32_t rcvMsgIdent;      /* identifier of the received message */
  CO_CANrx_t *buffer = NULL; /* receive message buffer from CO_CANmodule_t object. */
  bool_t msgMatched = false;

  esp_err_t ret = twai_receive(&rcvMsg, portMAX_DELAY);

  rcvMsgIdent = rcvMsg.identifier;
  if (CANmodule->useCANrxFilters)
  {
    /* CAN module filters are used. Message with known 11-bit identifier has */
    /* been received */

    // TODO: Hardare filters
    index = 0; /* get index of the received message here. Or something similar */
    // if (index < CANmodule->rxSize)
    // {
    //   buffer = &CANmodule->rxArray[index];
    //   /* verify also RTR */
    //   if (((rcvMsgIdent ^ buffer->ident) & buffer->mask) == 0U)
    //   {
    //     msgMatched = true;
    //   }
    // }
  }
  else
  {
    /* CAN module filters are not used, message with any standard 11-bit identifier */
    /* has been received. Search rxArray form CANmodule for the same CAN-ID. */
    buffer = &CANmodule->rxArray[0];
    for (index = CANmodule->rxSize; index > 0U; index--)
    {
      if (((rcvMsgIdent ^ buffer->ident) & buffer->mask) == 0U)
      {
        msgMatched = true;
        break;
      }
      buffer++;
    }
  }

  /* Call specific function, which will process the message */
  if (msgMatched && (buffer != NULL) && (buffer->CANrx_callback != NULL))
  {
    ESP_LOGI(CO_DRIVER_TAG, "twai_receive * ident: 0x%03X, DLC: %d 0x[%02X %02X %02X %02X %02X %02X %02X %02X] flags: 0x%08X idx: %d, cb: %d",
             (unsigned int) rcvMsg.identifier, rcvMsg.data_length_code, (unsigned int) rcvMsg.data[0], (unsigned int) rcvMsg.data[1], (unsigned int) rcvMsg.data[2], (unsigned int) rcvMsg.data[3],
             (unsigned int) rcvMsg.data[4], (unsigned int) rcvMsg.data[5], (unsigned int) rcvMsg.data[6], (unsigned int) rcvMsg.data[7], (unsigned int) rcvMsg.flags, CANmodule->rxSize - index,
             (int)(void *)buffer->CANrx_callback);

    buffer->CANrx_callback(buffer->object, (void *)&rcvMsg);
  }
  else
  {
    if (ret == ESP_OK)
    {
      ESP_LOGI(CO_DRIVER_TAG, "twai_receive ident: 0x%03X, DLC: %d 0x[%02X %02X %02X %02X %02X %02X %02X %02X] flags: 0x%08X",
               (unsigned int) rcvMsg.identifier, (unsigned int) rcvMsg.data_length_code, (unsigned int) rcvMsg.data[0], (unsigned int) rcvMsg.data[1], (unsigned int) rcvMsg.data[2], (unsigned int) rcvMsg.data[3],
               (unsigned int) rcvMsg.data[4], (unsigned int) rcvMsg.data[5], (unsigned int) rcvMsg.data[6], (unsigned int) rcvMsg.data[7], (unsigned int) rcvMsg.flags);
    }
  }
}

typedef struct
{
  uint16_t ident;
} CO_CANrxMsg_t;

// void CO_CANinterrupt(CO_CANmodule_t *CANmodule)
// {
//   ESP_LOGI(CO_DRIVER_TAG, "CO_CANinterrupt");

//   /* receive interrupt */
//   if (1)
//   {
//     CO_CANrxMsg_t *rcvMsg;     /* pointer to received message in CAN module */
//     uint16_t index;            /* index of received message */
//     uint32_t rcvMsgIdent;      /* identifier of the received message */
//     CO_CANrx_t *buffer = NULL; /* receive message buffer from CO_CANmodule_t object. */
//     bool_t msgMatched = false;

//     rcvMsg = 0; /* get message from module here */

//     rcvMsgIdent = rcvMsg->ident;
//     if (CANmodule->useCANrxFilters)
//     {
//       /* CAN module filters are used. Message with known 11-bit identifier has */
//       /* been received */
//       index = 0; /* get index of the received message here. Or something similar */
//       if (index < CANmodule->rxSize)
//       {
//         buffer = &CANmodule->rxArray[index];
//         /* verify also RTR */
//         if (((rcvMsgIdent ^ buffer->ident) & buffer->mask) == 0U)
//         {
//           msgMatched = true;
//         }
//       }
//     }
//     else
//     {
//       /* CAN module filters are not used, message with any standard 11-bit identifier */
//       /* has been received. Search rxArray form CANmodule for the same CAN-ID. */
//       buffer = &CANmodule->rxArray[0];
//       for (index = CANmodule->rxSize; index > 0U; index--)
//       {
//         if (((rcvMsgIdent ^ buffer->ident) & buffer->mask) == 0U)
//         {
//           msgMatched = true;
//           break;
//         }
//         buffer++;
//       }
//     }

//     /* Call specific function, which will process the message */
//     if (msgMatched && (buffer != NULL) && (buffer->CANrx_callback != NULL))
//     {
//       buffer->CANrx_callback(buffer->object, (void *)rcvMsg);
//     }

//     /* Clear interrupt flag */
//   }

//   /* transmit interrupt */
//   else if (0)
//   {
//     /* Clear interrupt flag */

//     /* First CAN message (bootup) was sent successfully */
//     CANmodule->firstCANtxMessage = false;
//     /* clear flag from previous message */
//     CANmodule->bufferInhibitFlag = false;
//     /* Are there any new messages waiting to be send */
//     if (CANmodule->CANtxCount > 0U)
//     {
//       uint16_t i; /* index of transmitting message */

//       /* first buffer */
//       CO_CANtx_t *buffer = &CANmodule->txArray[0];
//       /* search through whole array of pointers to transmit message buffers. */
//       for (i = CANmodule->txSize; i > 0U; i--)
//       {
//         /* if message buffer is full, send it. */
//         if (buffer->bufferFull)
//         {
//           buffer->bufferFull = false;
//           CANmodule->CANtxCount--;

//           /* Copy message to CAN buffer */
//           CANmodule->bufferInhibitFlag = buffer->syncFlag;
//           /* canSend... */
//           break; /* exit for loop */
//         }
//         buffer++;
//       } /* end of for loop */

//       /* Clear counter if no more messages */
//       if (i == 0U)
//       {
//         CANmodule->CANtxCount = 0U;
//       }
//     }
//   }
//   else
//   {
//     /* some other interrupt reason */
//   }
// }
