// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <cassert>
#include <algorithm>
#include <limits>

/** Set the chain tip to a given block index */
void CChain::SetTip(CBlockIndex* pindex) {
    if (!pindex) {
        vChain.clear();
        return;
    }
    vChain.resize(pindex->nHeight + 1);
    while (pindex && vChain[pindex->nHeight] != pindex) {
        vChain[pindex->nHeight] = pindex;
        pindex = pindex->pprev;
    }
}

/** Construct a block locator starting from a given block index (or the tip if none specified) */
CBlockLocator CChain::GetLocator(const CBlockIndex* pindex) const {
    std::vector<uint256> vHave;
    vHave.reserve(32);
    int nStep = 1;

    if (!pindex) pindex = Tip();

    while (pindex) {
        vHave.push_back(pindex->GetBlockHash());
        if (pindex->nHeight == 0) break;

        int nHeight = std::max(pindex->nHeight - nStep, 0);
        pindex = Contains(pindex) ? (*this)[nHeight] : pindex->GetAncestor(nHeight);

        if (vHave.size() > 10) nStep *= 2;
    }

    return CBlockLocator(vHave);
}

/** Find the fork point between this chain and another block index */
const CBlockIndex* CChain::FindFork(const CBlockIndex* pindex) const {
    if (!pindex) return nullptr;
    if (pindex->nHeight > Height())
        pindex = pindex->GetAncestor(Height());

    while (pindex && !Contains(pindex))
        pindex = pindex->pprev;

    return pindex;
}

/** Find the earliest block with time >= nTime */
CBlockIndex* CChain::FindEarliestAtLeast(int64_t nTime) const {
    auto lower = std::lower_bound(vChain.begin(), vChain.end(), nTime,
        [](CBlockIndex* pBlock, int64_t time) {
            return pBlock->GetBlockTimeMax() < time;
        });
    return (lower == vChain.end()) ? nullptr : *lower;
}

/** Internal utility: turn the lowest '1' bit in the binary representation of a number into '0' */
static inline int InvertLowestOne(int n) {
    return n & (n - 1);
}

/** Compute the skip height used for block index skip pointers */
static inline int GetSkipHeight(int height) {
    if (height < 2) return 0;
    return (height & 1) ?
        InvertLowestOne(InvertLowestOne(height - 1)) + 1 :
        InvertLowestOne(height);
}

/** Get ancestor block at specified height using skip pointers */
const CBlockIndex* CBlockIndex::GetAncestor(int height) const {
    if (height > nHeight || height < 0) return nullptr;

    const CBlockIndex* pindexWalk = this;
    int heightWalk = nHeight;

    while (heightWalk > height) {
        int heightSkip = GetSkipHeight(heightWalk);
        int heightSkipPrev = GetSkipHeight(heightWalk - 1);

        if (pindexWalk->pskip &&
            (heightSkip == height || 
            (heightSkip > height &&
             !(heightSkipPrev < heightSkip - 2 && heightSkipPrev >= height)))) {
            pindexWalk = pindexWalk->pskip;
            heightWalk = heightSkip;
        } else {
            assert(pindexWalk->pprev);
            pindexWalk = pindexWalk->pprev;
            --heightWalk;
        }
    }

    return pindexWalk;
}

CBlockIndex* CBlockIndex::GetAncestor(int height) {
    return const_cast<CBlockIndex*>(
        static_cast<const CBlockIndex*>(this)->GetAncestor(height)
    );
}

/** Build the skip pointer for this block */
void CBlockIndex::BuildSkip() {
    if (pprev) {
        pskip = pprev->GetAncestor(GetSkipHeight(nHeight));
    }
}

/** Calculate proof of a block (work representation) */
arith_uint256 GetBlockProof(const CBlockIndex& block) {
    arith_uint256 bnTarget;
    bool fNegative = false;
    bool fOverflow = false;

    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;

    return (~bnTarget / (bnTarget + 1)) + 1;
}

/** Get time difference between two blocks based on chainwork */
int64_t GetBlockProofEquivalentTime(const CBlockIndex& to, const CBlockIndex& from,
                                    const CBlockIndex& tip, const Consensus::Params& params) {
    arith_uint256 r;
    int sign = 1;

    if (to.nChainWork > from.nChainWork) {
        r = to.nChainWork - from.nChainWork;
    } else {
        r = from.nChainWork - to.nChainWork;
        sign = -1;
    }

    r = r * arith_uint256(params.nPowTargetSpacing) / GetBlockProof(tip);
    if (r.bits() > 63)
        return sign * std::numeric_limits<int64_t>::max();

    return sign * r.GetLow64();
}

/** Find the last common ancestor of two blocks */
const CBlockIndex* LastCommonAncestor(const CBlockIndex* pa, const CBlockIndex* pb) {
    assert(pa && pb);

    if (pa->nHeight > pb->nHeight)
        pa = pa->GetAncestor(pb->nHeight);
    else if (pb->nHeight > pa->nHeight)
        pb = pb->GetAncestor(pa->nHeight);

    while (pa != pb && pa && pb) {
        pa = pa->pprev;
        pb = pb->pprev;
    }

    assert(pa == pb); // Must converge to common ancestor
    return pa;
}
