// Copyright (c) 2012-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CHECKQUEUE_H
#define BITCOIN_CHECKQUEUE_H

#include <sync.h>

#include <algorithm>
#include <vector>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

/**
 * Queue for verifications that have to be performed.
 * T must provide an operator() that returns a bool.
 */
template <typename T>
class CCheckQueue {
private:
    boost::mutex mutex;
    boost::condition_variable condWorker;
    boost::condition_variable condMaster;

    std::vector<T> queue;
    int nIdle = 0;
    int nTotal = 0;
    bool fAllOk = true;
    unsigned int nTodo = 0;
    unsigned int nBatchSize;

    bool Loop(bool fMaster = false) {
        boost::condition_variable& cond = fMaster ? condMaster : condWorker;
        std::vector<T> vChecks;
        vChecks.reserve(nBatchSize);
        unsigned int nNow = 0;
        bool fOk = true;

        do {
            {
                boost::unique_lock<boost::mutex> lock(mutex);

                if (nNow) {
                    fAllOk &= fOk;
                    nTodo -= nNow;
                    if (nTodo == 0 && !fMaster)
                        condMaster.notify_one();
                } else {
                    nTotal++;
                }

                while (queue.empty()) {
                    if (fMaster && nTodo == 0) {
                        nTotal--;
                        bool fRet = fAllOk;
                        if (fMaster) fAllOk = true;
                        return fRet;
                    }
                    nIdle++;
                    cond.wait(lock);
                    nIdle--;
                }

                nNow = std::max(1U, std::min(nBatchSize, static_cast<unsigned int>(queue.size()) / (nTotal + nIdle + 1)));
                vChecks.resize(nNow);
                for (unsigned int i = 0; i < nNow; ++i) {
                    vChecks[i].swap(queue.back());
                    queue.pop_back();
                }

                fOk = fAllOk;
            }

            for (T& check : vChecks) {
                if (fOk)
                    fOk = check();
            }
            vChecks.clear();
        } while (true);
    }

public:
    boost::mutex ControlMutex;

    explicit CCheckQueue(unsigned int nBatchSizeIn) : nBatchSize(nBatchSizeIn) {}

    void Thread() {
        Loop();
    }

    bool Wait() {
        return Loop(true);
    }

    void Add(std::vector<T>& vChecks) {
        boost::unique_lock<boost::mutex> lock(mutex);
        for (T& check : vChecks) {
            queue.emplace_back();
            check.swap(queue.back());
        }
        nTodo += vChecks.size();
        if (vChecks.size() == 1)
            condWorker.notify_one();
        else if (!vChecks.empty())
            condWorker.notify_all();
    }

    ~CCheckQueue() = default;
};


/**
 * RAII-style controller for a CCheckQueue.
 * Guarantees the queue is finished before continuing.
 */
template <typename T>
class CCheckQueueControl {
private:
    CCheckQueue<T>* const pqueue;
    bool fDone = false;

public:
    CCheckQueueControl() = delete;
    CCheckQueueControl(const CCheckQueueControl&) = delete;
    CCheckQueueControl& operator=(const CCheckQueueControl&) = delete;

    explicit CCheckQueueControl(CCheckQueue<T>* const pqueueIn) : pqueue(pqueueIn) {
        if (pqueue) {
            ENTER_CRITICAL_SECTION(pqueue->ControlMutex);
        }
    }

    bool Wait() {
        if (!pqueue) return true;
        bool fRet = pqueue->Wait();
        fDone = true;
        return fRet;
    }

    void Add(std::vector<T>& vChecks) {
        if (pqueue) pqueue->Add(vChecks);
    }

    ~CCheckQueueControl() {
        if (!fDone)
            Wait();
        if (pqueue)
            LEAVE_CRITICAL_SECTION(pqueue->ControlMutex);
    }
};

#endif // BITCOIN_CHECKQUEUE_H
