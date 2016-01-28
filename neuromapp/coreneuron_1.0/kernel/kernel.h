/*
 * Neuromapp - kernel.h, Copyright (c), 2015,
 * Timothee Ewart - Swiss Federal Institute of technology in Lausanne,
 * timothee.ewart@epfl.ch,
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

/**
 * @file neuromapp/coreneuron_1.0/kernel/kernel.h
 * \brief Implements a miniapp on the compute kernel of coreneuron 1.0
 */

#ifndef MAPP_KERNEL_EXECUTE_
#define MAPP_KERNEL_EXECUTE_

#ifdef __cplusplus
     extern "C" {
#endif
    /** \fn coreneuron10_kernel_execute(int argc, char *const argv[])
        \brief miniapp reproducing the kernel of coreneuron 1.0
        \param argc number of argument from the command line
        \param argv the command line from the driver or external call
        \return error message from mapp::mapp_error
    */
    int coreneuron10_kernel_execute(int argc, char *const argv[]);
#ifdef __cplusplus
} //end extern "C"
#endif

#endif