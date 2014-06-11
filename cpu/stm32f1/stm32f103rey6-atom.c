/*
 * Copyright (C) 2013 INRIA
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_stm32f1
 * @{
 *
 * @file        stm32f103rey6-atom.c
 * @brief       Implementation of atomic set and return
 *
 * @author      Stefan Pfeiffer
 * @author      Alaeddine Weslati <alaeddine.weslati@inria.fr>
 *
 * @}
 */
#include "cpu.h"

unsigned int atomic_set_return(unsigned int *p, unsigned int uiVal)
{
    //unsigned int cspr = disableIRQ();		//crashes
    dINT();
    unsigned int uiOldVal = *p;
    *p = uiVal;
    //restoreIRQ(cspr);						//crashes
    eINT();
    return uiOldVal;
}
