/********************************************************************************
 *    Copyright (C) 2019 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *              GNU Lesser General Public Licence (LGPL) version 3,             *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/

#ifndef FAIR_MQ_TOOLS_SEMAPHORE_H
#define FAIR_MQ_TOOLS_SEMAPHORE_H

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>

namespace fair {
namespace mq {
namespace tools {

/**
 * @struct Semaphore Semaphore.h <fairmq/tools/Semaphore.h>
 * @brief A simple blocking semaphore.
 */
struct Semaphore
{
  explicit Semaphore();
  explicit Semaphore(std::size_t initial_count);

  auto Wait() -> void;
  auto Signal() -> void;
  auto GetCount() -> std::size_t;

private:
  std::size_t fCount;
  std::mutex fMutex;
  std::condition_variable fCv;
};

} /* namespace tools */
} /* namespace mq */
} /* namespace fair */

#endif /* FAIR_MQ_TOOLS_SEMAPHORE_H */