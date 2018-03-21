// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "chainparams.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"

#include <math.h>

/** TERRACOIN previous POWs **/
unsigned int GetNextWorkRequiredV2(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    int64_t retargetBlockCountInterval = 2160;
    int64_t lookupBlockCount = 2160;

    if (pindexLast->nHeight > 192237) {
        retargetBlockCountInterval = 540; // retarget every 540 blocks
        lookupBlockCount = 540; // past blocks to use for timing
    }

    const int64_t retargetTimespan = 120 * retargetBlockCountInterval; // 2 minutes per block
    const int64_t retargetVsInspectRatio = lookupBlockCount / retargetBlockCountInterval; // currently 12

    // non-retargetting block: keep same diff:
    if ((pindexLast->nHeight+1) % retargetBlockCountInterval != 0 || (pindexLast->nHeight < lookupBlockCount)) {
        return (pindexLast->nBits);
    }

    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(pindexLast->nHeight - lookupBlockCount);
    assert(pindexFirst);

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    nActualTimespan = nActualTimespan / retargetVsInspectRatio;

    if (pindexLast->nHeight > 192237) {
        // at and after block 192240, use 1.25 limits:
        if (nActualTimespan < retargetTimespan / 1.25) {
            nActualTimespan = retargetTimespan / 1.25;
        }
        if (nActualTimespan > retargetTimespan * 1.25) {
            nActualTimespan = retargetTimespan * 1.25;
        }
    } else {
        if (nActualTimespan < retargetTimespan / 4) {
            nActualTimespan = retargetTimespan / 4;
        }
        if (nActualTimespan > retargetTimespan * 4) {
            nActualTimespan = retargetTimespan * 4;
        }
    }

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= retargetTimespan;

    // during the switchover from EMA retargetting to static 2160 retargetting:
    // temporary, low diff limit: 17.4k self deactivating at height 182000
    // (for first retarget only)
    if (pindexLast->nHeight < 183000) {
        arith_uint256 seventeenThousandsLimit;
        seventeenThousandsLimit.SetCompact(0x1b03bf8b);
        if (bnNew > seventeenThousandsLimit) {
            bnNew = seventeenThousandsLimit;
        }
    }

    // temporary, super ugly way to never, ever return diff < 5254,
    // just in the case something really bad happens
    // self-deactivate at block 220000
    if (pindexLast->nHeight < 220000) {
        arith_uint256 fiveThousandsLimit;
        fiveThousandsLimit.SetCompact(0x1b0c7898);
        if (bnNew > fiveThousandsLimit) {
            bnNew = fiveThousandsLimit;
        }
    }

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}
unsigned int GetNextWorkRequiredEMA(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();
    const int64_t perBlockTargetTimespan = 120; // two mins between blocks
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    int64_t block_durations[2160];
    float alpha = 0.09; // closer to 1.0 = faster response to new values
    if (pindexLast->nHeight > 110322) {
        alpha = 0.06;
    }
    float accumulator = 120;
    arith_uint256 fiveThousandsLimit;
    fiveThousandsLimit.SetCompact(0x1b0c7898);

    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    if (pindexLast->nHeight < 175000 && pblock->nTime > (pindexLast->nTime + perBlockTargetTimespan*10)) {
	arith_uint256 bnNew;
	bnNew.SetCompact(pindexLast->nBits);
	if (pindexLast->nHeight > 101631 && pindexLast->nHeight < 103791) {
	    LogPrintf("GetNextWorkRequiredEMA Inflated Diff\n");
	    bnNew *= 10;
	} else {
	    bnNew *= 2;
	}

	// super ugly way to never, ever return diff < 5254:
	if (pindexLast->nHeight > 104290) {
	    if (bnNew > fiveThousandsLimit) {
		bnNew = fiveThousandsLimit;
	    }
	}


	if (bnNew > bnPowLimit)
	    bnNew = bnPowLimit;
        LogPrintf("GetNextWorkRequiredEMA RETARGET\n");
	return bnNew.GetCompact();
    }

    // collect last 3 days (30*24*3=2160) blocks durations:
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < 2160 ; i++) {
        block_durations[2159 - i] = pindexFirst->GetBlockTime() - pindexFirst->pprev->GetBlockTime();

        if (pindexLast->nHeight > 110322) {
            // slow down difficulty decrease even more,
            // also limit the effect of future nTime values (actually annihilates them):
            if (block_durations[2159 - i] > (1.5 * perBlockTargetTimespan) ) {
                block_durations[2159 - i] = 1.5 * perBlockTargetTimespan;
            }

            // slow down difficulty increase:
            if ((block_durations[2159 - i] >= 0) && (block_durations[2159 - i] < (perBlockTargetTimespan / 2)) ) {
                block_durations[2159 - i] = perBlockTargetTimespan / 2;
            }
        }

        if (block_durations[2159 - i] < 0 && pindexLast->nHeight > 104290) {
            block_durations[2159 - i] = perBlockTargetTimespan;
        }
        pindexFirst = pindexFirst->pprev;
    }

    // compute exponential moving average block duration:
    for (int i=0; i<2160 ; i++) {
        accumulator = (alpha * block_durations[i]) + (1 - alpha) * accumulator;
    }

    int64_t nActualTimespan = accumulator;
    if (nActualTimespan < perBlockTargetTimespan / 2)
        nActualTimespan = perBlockTargetTimespan / 2;

    if (pindexLast->nHeight > 110322 && nActualTimespan > perBlockTargetTimespan * 2) {
        nActualTimespan = perBlockTargetTimespan * 2;
    } else if(nActualTimespan > perBlockTargetTimespan * 4) {
        nActualTimespan = perBlockTargetTimespan * 4;
    }

    // Retarget
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= perBlockTargetTimespan;


    // temporary, super ugly way to never, ever return diff < 5254:
    if (pindexLast->nHeight > 104290) {
        if (bnNew > fiveThousandsLimit) {
            bnNew = fiveThousandsLimit;
        }
    }

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;
    LogPrintf("GetNextWorkRequiredEMA RETARGET\n");
    return bnNew.GetCompact();
}
/** END TERRACOIN previous POWs **/

unsigned int static KimotoGravityWell(const CBlockIndex* pindexLast, const Consensus::Params& params) {
    const CBlockIndex *BlockLastSolved = pindexLast;
    const CBlockIndex *BlockReading = pindexLast;
    uint64_t PastBlocksMass = 0;
    int64_t PastRateActualSeconds = 0;
    int64_t PastRateTargetSeconds = 0;
    double PastRateAdjustmentRatio = double(1);
    arith_uint256 PastDifficultyAverage;
    arith_uint256 PastDifficultyAveragePrev;
    double EventHorizonDeviation;
    double EventHorizonDeviationFast;
    double EventHorizonDeviationSlow;

    uint64_t pastSecondsMin = params.nPowTargetTimespan * 0.025;
    uint64_t pastSecondsMax = params.nPowTargetTimespan * 7;
    uint64_t PastBlocksMin = pastSecondsMin / params.nPowTargetSpacing;
    uint64_t PastBlocksMax = pastSecondsMax / params.nPowTargetSpacing;

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || (uint64_t)BlockLastSolved->nHeight < PastBlocksMin) { return UintToArith256(params.powLimit).GetCompact(); }

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
        PastBlocksMass++;

        PastDifficultyAverage.SetCompact(BlockReading->nBits);
        if (i > 1) {
            // handle negative arith_uint256
            if(PastDifficultyAverage >= PastDifficultyAveragePrev)
                PastDifficultyAverage = ((PastDifficultyAverage - PastDifficultyAveragePrev) / i) + PastDifficultyAveragePrev;
            else
                PastDifficultyAverage = PastDifficultyAveragePrev - ((PastDifficultyAveragePrev - PastDifficultyAverage) / i);
        }
        PastDifficultyAveragePrev = PastDifficultyAverage;

        PastRateActualSeconds = BlockLastSolved->GetBlockTime() - BlockReading->GetBlockTime();
        PastRateTargetSeconds = params.nPowTargetSpacing * PastBlocksMass;
        PastRateAdjustmentRatio = double(1);
        if (PastRateActualSeconds < 0) { PastRateActualSeconds = 0; }
        if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
            PastRateAdjustmentRatio = double(PastRateTargetSeconds) / double(PastRateActualSeconds);
        }
        EventHorizonDeviation = 1 + (0.7084 * pow((double(PastBlocksMass)/double(28.2)), -1.228));
        EventHorizonDeviationFast = EventHorizonDeviation;
        EventHorizonDeviationSlow = 1 / EventHorizonDeviation;

        if (PastBlocksMass >= PastBlocksMin) {
                if ((PastRateAdjustmentRatio <= EventHorizonDeviationSlow) || (PastRateAdjustmentRatio >= EventHorizonDeviationFast))
                { assert(BlockReading); break; }
        }
        if (BlockReading->pprev == NULL) { assert(BlockReading); break; }
        BlockReading = BlockReading->pprev;
    }

    arith_uint256 bnNew(PastDifficultyAverage);
    if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
        bnNew *= PastRateActualSeconds;
        bnNew /= PastRateTargetSeconds;
    }

    if (bnNew > UintToArith256(params.powLimit)) {
        bnNew = UintToArith256(params.powLimit);
    }

    return bnNew.GetCompact();
}

unsigned int static DarkGravityWave(const CBlockIndex* pindexLast, const Consensus::Params& params) {
    /* current difficulty formula, dash - DarkGravity v3, written by Evan Duffield - evan@dash.org */
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    int64_t nPastBlocks = 24;

    // make sure we have at least (nPastBlocks + 1) blocks, otherwise just return powLimit
    if (!pindexLast || pindexLast->nHeight < nPastBlocks) {
        return bnPowLimit.GetCompact();
    }

    const CBlockIndex *pindex = pindexLast;
    arith_uint256 bnPastTargetAvg;

    for (unsigned int nCountBlocks = 1; nCountBlocks <= nPastBlocks; nCountBlocks++) {
        arith_uint256 bnTarget = arith_uint256().SetCompact(pindex->nBits);
        if (nCountBlocks == 1) {
            bnPastTargetAvg = bnTarget;
        } else {
            // NOTE: that's not an average really...
            bnPastTargetAvg = (bnPastTargetAvg * nCountBlocks + bnTarget) / (nCountBlocks + 1);
        }

        if(nCountBlocks != nPastBlocks) {
            assert(pindex->pprev); // should never fail
            pindex = pindex->pprev;
        }
    }

    arith_uint256 bnNew(bnPastTargetAvg);

    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindex->GetBlockTime();
    // NOTE: is this accurate? nActualTimespan counts it for (nPastBlocks - 1) blocks only...
    int64_t nTargetTimespan = nPastBlocks * params.nPowTargetSpacing;

    if (nActualTimespan < nTargetTimespan/3)
        nActualTimespan = nTargetTimespan/3;
    if (nActualTimespan > nTargetTimespan*3)
        nActualTimespan = nTargetTimespan*3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

    if (bnNew > bnPowLimit) {
        bnNew = bnPowLimit;
    }

    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequiredBTC(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // Only change once per interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 2.5 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 1 hourworth of blocks
    // TERRACOIN START
    // int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    // assert(nHeightFirst >= 0);
    // const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    // assert(pindexFirst);
    int nBlocksLookupRange = params.DifficultyAdjustmentInterval() - 1;
    if (pindexLast->nHeight > 99988) {
	nBlocksLookupRange = params.DifficultyAdjustmentInterval() * 24;
    }
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(pindexLast->nHeight - nBlocksLookupRange);
    assert(pindexFirst);
    // TERRACOIN END

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    // Exception first
    if (pindexLast->nHeight == 137161 && Params().NetworkIDString() == CBaseChainParams::MAIN) {
        return (0x1b034c51);
    }

    // Most recent algo first
    if (pindexLast->nHeight + 1 >= params.nPowDGWHeight) {
        return DarkGravityWave(pindexLast, params);
    }
    else if (pindexLast->nHeight + 1 >= params.nPowKGWHeight) {
        return KimotoGravityWell(pindexLast, params);
    }
    else if (pindexLast->nHeight + 1 >= params.nPowV2Height) {
        return GetNextWorkRequiredV2(pindexLast, pblock, params);
    }
    else if (pindexLast->nHeight + 1 >= params.nPowEMAHeight) {
        return GetNextWorkRequiredEMA(pindexLast, pblock, params);
    }
    else {
        return GetNextWorkRequiredBTC(pindexLast, pblock, params);
    }
}

// for DIFF_BTC only!
unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    // TERRACOIN START
    //if (params.fPowNoRetargeting)
    //    return pindexLast->nBits;
    // TERRACOIN END

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    // TERRACOIN START
    if (pindexLast->nHeight > 101908) {
        nActualTimespan = nActualTimespan / 3;
    } else if (pindexLast->nHeight > 99988) {
        nActualTimespan = nActualTimespan / 24;
    }
    // TERRACOIN END
    LogPrintf("  nActualTimespan = %d  before bounds\n", nActualTimespan);
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    arith_uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    /// debug print
    LogPrintf("GetNextWorkRequired RETARGET\n");
    LogPrintf("params.nPowTargetTimespan = %d    nActualTimespan = %d\n", params.nPowTargetTimespan, nActualTimespan);
    LogPrintf("Before: %08x  %s\n", pindexLast->nBits, bnOld.ToString());
    LogPrintf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.ToString());

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return error("CheckProofOfWork(): nBits below minimum work");

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return error("CheckProofOfWork(): hash doesn't match nBits");

    return true;
}

arith_uint256 GetBlockProof(const CBlockIndex& block)
{
    arith_uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a arith_uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
}

int64_t GetBlockProofEquivalentTime(const CBlockIndex& to, const CBlockIndex& from, const CBlockIndex& tip, const Consensus::Params& params)
{
    arith_uint256 r;
    int sign = 1;
    if (to.nChainWork > from.nChainWork) {
        r = to.nChainWork - from.nChainWork;
    } else {
        r = from.nChainWork - to.nChainWork;
        sign = -1;
    }
    r = r * arith_uint256(params.nPowTargetSpacing) / GetBlockProof(tip);
    if (r.bits() > 63) {
        return sign * std::numeric_limits<int64_t>::max();
    }
    return sign * r.GetLow64();
}
