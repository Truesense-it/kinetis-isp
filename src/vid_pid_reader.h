/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#ifndef __VID_PID_READER_H_
#define __VID_PID_READER_H_

#include <tuple>
#include <string>

namespace VIDPIDReader{
    std::tuple<int, int> getVidPidForDev(std::string dev);
}


#endif /* __VID_PID_READER_H_ */