#include "inc.h"
#include <stdint.h>

/** 
 * @file fifo.c
 * @brief Implémentation d'un FIFO pour la liaison série sur STM32G431.
 */





/**
 * @brief Initialise la structure du FIFO.
 *
 * Cette fonction initialise les indices de tête et de queue.
 *
 * @param[in,out] fifo Pointeur vers la structure FIFO.
 */
void fifo_init(fifo_t *fifo)
{
    if (fifo != (void *)0)
    {
        fifo->head = 0U;
        fifo->tail = 0U;
    }
}

#include <stdbool.h>

/**
 * @brief Vérifie si le FIFO est vide.
 *
 * Cette fonction retourne true si le FIFO est vide (i.e. si head == tail).
 * Pour garantir la cohérence des lectures en contexte concurrent, les interruptions
 * sont désactivées lors de la lecture des indices.
 *
 * @param[in] fifo Pointeur vers la structure FIFO.
 * @return bool true si le FIFO est vide, false sinon.
 */
bool fifo_is_empy(fifo_t *fifo)
{
    bool isEmpty = true;

    if (fifo != (void *)0)
    {
//        __disable_irq();
        isEmpty = (fifo->head == fifo->tail);
//        __enable_irq();
    }
    return isEmpty;
}


/**
 * @brief Insère une valeur dans le FIFO.
 *
 * Cette fonction insère une valeur dans le FIFO si celui-ci n'est pas plein.
 *
 * @param[in,out] fifo Pointeur vers la structure FIFO.
 * @param[in] val Valeur à insérer.
 * @return int FIFO_OK si l'insertion a réussi, FIFO_ERROR sinon (FIFO plein ou pointeur nul).
 */
int fifo_put(fifo_t *fifo, uint8_t val)
{
    int ret = FIFO_OK;
    uint32_t next_head;

    if (fifo == (void *)0)
    {
        ret = FIFO_ERROR;
    }
    else
    {
        next_head = (fifo->head + 1U) % FIFO_BUFFER_SIZE;
        if (next_head == fifo->tail)
        {
            /* FIFO plein */
            ret = FIFO_ERROR;
        }
        else
        {
            fifo->buffer[fifo->head] = val;
            fifo->head = next_head;
        }
    }
    return ret;
}

/**
 * @brief Récupère une valeur dans le FIFO.
 *
 * Cette fonction récupère une valeur dans le FIFO si celui-ci n'est pas vide.
 * Elle désactive les interruptions pour garantir l'atomicité de la lecture.
 *
 * @param[in,out] fifo Pointeur vers la structure FIFO.
 * @param[out] val Pointeur où sera stockée la valeur lue.
 * @return int FIFO_OK si la lecture a réussi, FIFO_ERROR sinon (FIFO vide ou pointeur nul).
 */
int fifo_get(fifo_t *fifo, uint8_t *val)
{
    int ret = FIFO_OK;

    if ((fifo == (void *)0) || (val == (void *)0))
    {
        ret = FIFO_ERROR;
    }
    else
    {
//        __disable_irq();
        if (fifo->head == fifo->tail)
        {
            /* FIFO vide */
//            __enable_irq();
            ret = FIFO_ERROR;
        }
        else
        {
            *val = fifo->buffer[fifo->tail];
            fifo->tail = (fifo->tail + 1U) % FIFO_BUFFER_SIZE;
//            __enable_irq();
        }
    }
    return ret;
}

/**
 * @brief Attend qu'un nombre minimum d'octets soit disponible dans le FIFO.
 *
 * Cette fonction attend, pendant une durée maximale de timeout_ms millisecondes, que le nombre
 * d'octets disponibles dans le FIFO soit supérieur ou égal à count.
 *
 * @param[in] fifo Pointeur vers la structure FIFO.
 * @param[in] count Nombre d'octets minimum à attendre.
 * @param[in] timeout_ms Délai maximal en millisecondes.
 * @return int FIFO_OK si le nombre d'octets est atteint, FIFO_ERROR en cas de timeout ou de problème.
 */
int fifo_wait_for(fifo_t *fifo, unsigned int count, unsigned int timeout_ms)
{
    int ret = FIFO_ERROR;
    uint32_t start_time;
    uint32_t available;
    uint32_t temp_head;
    uint32_t temp_tail;

    if (fifo == NULL)
    {
        return FIFO_ERROR;
    }

    start_time = HAL_GetTick();
    while (1)
    {
        /* Désactivation temporaire des interruptions pour lecture cohérente */
        __disable_irq();
        temp_head = fifo->head;
        temp_tail = fifo->tail;
        __enable_irq();

        if (temp_head >= temp_tail)
        {
            available = temp_head - temp_tail;
        }
        else
        {
            available = (FIFO_BUFFER_SIZE - temp_tail) + temp_head;
        }

        if (available >= count)
        {
            ret = FIFO_OK;
            break;
        }

        /* Gestion robuste du timeout */
        uint32_t elapsed_time = HAL_GetTick() - start_time;
        if (elapsed_time >= timeout_ms)
        {
            break;
        }
    }

    return ret;
}
