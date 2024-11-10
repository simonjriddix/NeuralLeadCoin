// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/merkle.h>
#include <consensus/consensus.h>
#include <hash.h> // for signet block challenge hash
#include <tinyformat.h>
#include <util/system.h>
#include <util/strencodings.h>
#include <versionbitsinfo.h>
#include <miner.h>

#include <assert.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>


static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "It is not crucial how high you aim but that you never reach it"; // because when you reach a goal, nothing else matters, and everything fades away.
    const CScript genesisOutputScript = CScript() << ParseHex("04f464d5dcb028f26ace786ad7642b8bb6324bbbd577644799d7f71ac4fdbf00c7144afddf02fb7055c0c4f057ffefd3783cee56f7ae7d6fd44e8a21ebda748deb") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

int mineNow(std::string mode, uint32_t nTime, Consensus::Params& consensus, CBlock& genesis, uint32_t nB, std::string FileName = "")
{
    size_t i = 0;
    size_t diffragg = 0;
    size_t minDiff = 3;

    bool UseFile = FileName!="";
    std::ofstream outfile (FileName);

        while(true)
        {
            genesis = CreateGenesisBlock(nTime, i, nB, 1, 60 * COIN);
            consensus.hashGenesisBlock = genesis.GetHash();

            auto hb = consensus.hashGenesisBlock.ToString();
            
            size_t y = 0;
            for ( ; y < hb.size(); y++)
            {
                if (hb[y] != '0')
                {
                    break;
                }
            }
            if(y > diffragg)
            {
                diffragg = y;
                
                if(diffragg >= minDiff)
                {
                    break;
                }
            }
            
            if(i % 100 == 0)
            {
                printf("%s nonce %ld\t\tdiffragg %ld\n", mode.c_str(), i, diffragg);
                printf("genesis %s\n", consensus.hashGenesisBlock.ToString().c_str());
                printf("markle  %s\n", genesis.hashMerkleRoot.ToString().c_str());
                
                if(UseFile)
                {
                    outfile << "genesis " << consensus.hashGenesisBlock.ToString().c_str() << std::endl;
                    outfile << "Merkle " << genesis.hashMerkleRoot.ToString().c_str() << std::endl;
                    outfile << "nonce " << std::to_string(i) << "\t\tdiffragg" << std::to_string(diffragg) << std::endl;
                    outfile << std::endl;
                }
            }

            bool fNegative;
            bool fOverflow;
            arith_uint256 bnTarget;

            bnTarget.SetCompact(nB, &fNegative, &fOverflow);

            // Check range
            if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(consensus.powLimit))
            {

            }
            // Check proof of work matches claimed amount
            else if (bnTarget > UintToArith256(genesis.GetHash()))
            {
                break;
            }

            i++;
        }
        
        if(UseFile)
        {
            outfile << "genesis " << consensus.hashGenesisBlock.ToString().c_str() << std::endl;
            outfile << "Merkle " << genesis.hashMerkleRoot.ToString().c_str() << std::endl;
            outfile << "nonce " << std::to_string(i) << "\t\tdiffragg" << std::to_string(diffragg) << std::endl;
            outfile.close();
        }

        printf("%s\n", mode.c_str());
        printf("nonce %ld\t\tdiffragg %ld\n", i, diffragg);
        printf("genesis %s\n", consensus.hashGenesisBlock.ToString().c_str());
        printf("markle  %s\n", genesis.hashMerkleRoot.ToString().c_str());
        printf("\n");

        return diffragg;
}

static int NormalizeRewards(Consensus::Params& consensus)
{
    // Calculate the coin Value now to avoid continuosly recalculation in code
    consensus.PoSRewardCoins_Normalized = (consensus.PoSRewardCoins * COIN);

    int percToTeam = 0;

    // Normalize % for team wallets
    std::map<std::string, int>::iterator it_prec = consensus.rewardExtraAddresses.begin();
    while (it_prec != consensus.rewardExtraAddresses.end()) {
        //it_prec->second /= 100.0;
        percToTeam += it_prec->second;
        ++it_prec;
    }

    return percToTeam;
}
#include <pow.h>
/**
 * Main network
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        int newBlockMins = 6; // mine new block every (avg) minutes

        strNetworkID = CBaseChainParams::MAIN;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 350400; // avg. 4 yrs
        consensus.BIP16Exception = uint256S("0x00000bcd2d9ccbb28606a8b2d962b97394f612bf6e021ce1d64d71cecb008029");
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x00000b3b403a2b37780a8dc3813e02463cbaceab135efffe4aaacf0446f862d5");
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 1;
        consensus.nMinerConfirmationWindow = 2016;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.MinBIP9WarningHeight = consensus.SegwitHeight + consensus.nMinerConfirmationWindow;
        consensus.BPSRewardMatchStep = 350400; // PoS Reward Halving avg. 4 yrs
        consensus.BPSRewardMatchHeight = newBlockMins * consensus.BPSRewardMatchStep;
        consensus.BPSDiffAdjHeight = 10000; // start to Adjust difficulty
        consensus.powLimit = uint256S("000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("0000000fffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 10 * newBlockMins * 60; // every 10 blocks
        consensus.nPowTargetSpacing = newBlockMins * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.fPoSNoRetargeting = false;
        
        consensus.nMPoSRewardRecipients = 10;
        consensus.EnableStackingAtBlock = COINBASE_MATURITY + 1; // Switch from PoW to PoS and enable Cashback / AntiStall

        consensus.rewardExtraAddresses = {
            {"nEp6ET5SYUbRPx8ASzauYVxLhKgyYuBoaH", 5}, // %      Developers
            {"nMqxZ4qcN3Sem6AEMkbqXpNHCMJVX1PRcn", 8}, // %      R&D
            {"nMYhdmvgBG9pjXxzdL3GTpwt4JAc1EZerX", 6}, // %      DataMining
            {"nRzFNpNPK61uZYmPcUU2PfVDBa28YBiPa7", 2}, // %      Marketing 2.0
            
            // Not Team
            {"nKFtazUd2WAxoKjnq3wSYr7oq1Tbi6zUnK", 4}, // %      AntiStall & CashBack Address
        };
        consensus.plywoodRewardAddress = "nLc3DXwMsxqsYas2cTWhrfq1d5jgaLr1uL"; // dyn R&D, Dev, Marketing, AirDrop...
        consensus.AirDropRewardAddress = "n6hazxacLTZm1V9GbWV3Lj7QQ7fqYDrps4"; // AirDrop
        
        consensus.AntiStallAddress = "nKFtazUd2WAxoKjnq3wSYr7oq1Tbi6zUnK"; // AntiStall & CashBack Address

        consensus.PoSRewardCoins = 60;
        consensus.PoSMinimumBalanceToStake = 300 * COIN; // Is Little investor

        consensus.littleStakerPercentage = 7; // 7 %
        consensus.littleStakerMinimumCoins = consensus.PoSMinimumBalanceToStake;

        consensus.mediumStakerPercentage = 23; // 23 %
        consensus.mediumStakerMinimumCoins = 1100 * COIN;

        consensus.bigStakerPercentage = 62; // 62 %
        consensus.bigStakerMinimumCoins = 3100 * COIN;

        consensus.minimumStakerPercentage = 0.00; // 0 %, Under Little investor you can't stake, this is mean you are try to hack, put reward for that hacked block to 0
        
        consensus.nEnableHeaderSignatureHeight = 0;
        consensus.nCheckpointSpan = COINBASE_MATURITY;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = 1230767999; // December 31, 2008

        //consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000074e123470d6bcaa7c"); //block 236,425
        //consensus.defaultAssumeValid = uint256S("0xf24b2bc2f0476da8a42042610524991c66871ae065dac162b47de60ac379f934"); //block 236,425
        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000001"); //block 1
        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); //block 1

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x4E; // N
        pchMessageStart[1] = 0x4C; // L
        pchMessageStart[2] = 0x43; // C
        pchMessageStart[3] = 0x31; // 1
        nDefaultPort = 10101;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 2;
        m_assumed_chain_state_size = 1;
        
        uint32_t nB_difficulty = 0x1F0fffff;

        //int noncce = mineNow("[Main]", 1731067200, consensus, genesis, nB_difficulty, "gen-main.txt"); exit(noncce);

        genesis = CreateGenesisBlock(1731067200, 2157, nB_difficulty, 1, 60 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        std::cout << genesis.GetHash().ToString()<< std::endl;

        assert(consensus.hashGenesisBlock == uint256S("0x000d5fdb362c87818f82b4e1979faef9f19ba06e400f0de1a8530ae51a7cd2f9"));
        assert(genesis.hashMerkleRoot == uint256S("0x29b3179534725ed9935e4d86b54f9f8b47894042dfbcf8d860e036100b2b493f"));

        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as an addrfetch if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.
        vSeeds.emplace_back("75.119.146.38");
        vSeeds.emplace_back("46.250.250.218");
        vSeeds.emplace_back("195.32.8.201");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 112); // n
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 48);  // L
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1, 128); //  
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};               
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};               

        bech32_hrp = "NLEAD";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = false;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                { 0, uint256S("0x000d5fdb362c87818f82b4e1979faef9f19ba06e400f0de1a8530ae51a7cd2f9")}, 
            }
        };

        chainTxData = ChainTxData{
            // Data from RPC: getchaintxstats
            /* nTime    */ 1731067200,
            /* nTxCount */ 0,
            /* dTxRate  */ 0,
        };

        int percToTeam = NormalizeRewards(consensus);

        if(COLD_STAKER_FEE > 100 || COLD_STAKER_FEE < 0)
        {
            LogPrintf("invalid COLD_STAKER_FEE");
            throw std::runtime_error("invalid COLD_STAKER_FEE");
        }

        if(UNLOCK_PoW_COINS_AFTER_BLOCKS < COINBASE_MATURITY)
        {
            LogPrintf("UNLOCK_PoW_COINS_AFTER_BLOCKS can't be with a value under COINBASE_MATURITY");
            throw std::runtime_error("UNLOCK_PoW_COINS_AFTER_BLOCKS can't be with a value under COINBASE_MATURITY");
        }

        if(UNLOCK_PoS_COINS_AFTER_BLOCKS < COINBASE_MATURITY)
        {
            LogPrintf("UNLOCK_PoS_COINS_AFTER_BLOCKS can't be with a value under COINBASE_MATURITY");
            throw std::runtime_error("UNLOCK_PoS_COINS_AFTER_BLOCKS can't be with a value under COINBASE_MATURITY");
        }

        if(consensus.rewardExtraAddresses.size() > 0 && (percToTeam <= 0 || percToTeam > 100))
        {
            LogPrintf("percentage for wallet addresses in rewardExtraAddresses are wrong");
            throw std::runtime_error("percentage for wallet addresses in rewardExtraAddresses are wrong");
        }

        if(consensus.nMPoSRewardRecipients < 2)
        {
            LogPrintf("nMPoSRewardRecipients is too low at least 2");
            throw std::runtime_error("nMPoSRewardRecipients is too low at least 2");
        }
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = CBaseChainParams::TESTNET;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 350400;
        consensus.BIP16Exception = uint256S("0x00000fe2acf48e35c5b594d9ff7db2a7bbafa1b73205b2789a6833be70595818");
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x00000fe2acf48e35c5b594d9ff7db2a7bbafa1b73205b2789a6833be70595818");
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 1;
        consensus.MinBIP9WarningHeight = consensus.SegwitHeight + consensus.nMinerConfirmationWindow;
        consensus.BPSRewardMatchStep = 400;
        consensus.BPSRewardMatchHeight = 3 * consensus.BPSRewardMatchStep;
        consensus.BPSDiffAdjHeight = 1500;
        consensus.EnableStackingAtBlock = 2000;
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 10 * 3 * 60; // every 10 blocks
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.fPoSNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.nMPoSRewardRecipients = 10;

        consensus.PoSRewardCoins = 50;

        consensus.rewardExtraAddresses = {
            {"UgD6oyq8Rxn66D81t28TBfvxXsQGCp1jSn", 1},      // Team
            {"UVuXQH5raXQK7RL2nkgXDtGTfEtCQNc8y5", 1},     // Community
        };

        consensus.littleStakerPercentage = 0.2;
        consensus.littleStakerMinimumCoins = 100 * COIN;

        consensus.mediumStakerPercentage = 0.4;
        consensus.mediumStakerMinimumCoins = 250 * COIN;

        consensus.bigStakerPercentage = 0.9;
        consensus.bigStakerMinimumCoins = 1000 * COIN;

        consensus.minimumStakerPercentage = 0.05; // 5 %

        consensus.plywoodRewardAddress = "abc"; // AirDrop & Marketing
        consensus.AirDropRewardAddress = ""; // AirDrop
        
        consensus.nEnableHeaderSignatureHeight = 0;
        consensus.nCheckpointSpan = COINBASE_MATURITY;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = 1230767999; // December 31, 2008

        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000"); // 0
        consensus.defaultAssumeValid = uint256S("0x00000fe2acf48e35c5b594d9ff7db2a7bbafa1b73205b2789a6833be70595818"); // 0

        pchMessageStart[0] = 0xd5;
        pchMessageStart[1] = 0x1f;
        pchMessageStart[2] = 0x35;
        pchMessageStart[3] = 0x29;
        nDefaultPort = 48932;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 40;
        m_assumed_chain_state_size = 2;

        genesis = CreateGenesisBlock(1588417200, 3152477, 0x1e0fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        uint32_t nB = 0x1e0fffff;
        //mineNow("[TestNet]", consensus, genesis, nB);
        //assert(consensus.hashGenesisBlock == uint256S("0x00000fe2acf48e35c5b594d9ff7db2a7bbafa1b73205b2789a6833be70595818"));
        //assert(genesis.hashMerkleRoot == uint256S("0xb44e2d41890cc021a91405d7944b77ac4a27fadfc0caa9734c68fc64da09a207"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.emplace_back("testnet-seed1.neurallead.com");
        vSeeds.emplace_back("testnet-seed2.neurallead.com");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,65);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,78);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,130);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "nlt";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        m_is_test_chain = true;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                //{0, uint256S("0x00000fe2acf48e35c5b594d9ff7db2a7bbafa1b73205b2789a6833be70595818")},
                { 0, consensus.hashGenesisBlock },
            }
        };

        chainTxData = ChainTxData{
            // Data from RPC: getchaintxstats 4096 00000000000000b7ab6ce61eb6d571003fbe5fe892da4c9b740c49a07542462d
            /* nTime    */ 1588417200,
            /* nTxCount */ 0,
            /* dTxRate  */ 0.0,
        };

        NormalizeRewards(consensus);
    }
};

/**
 * Signet
 */
class SigNetParams : public CChainParams {
public:
    explicit SigNetParams(const ArgsManager& args) {
        std::vector<uint8_t> bin;
        vSeeds.clear();

        if (!args.IsArgSet("-signetchallenge")) {
            bin = ParseHex("512103ad5e0edad18cb1f0fc0d28a3d4f1f3e445640337489abb10404f2d1e086be430210359ef5021964fe22d6f8e05b2463c9540ce96883fe3b278760f048f5189f2e6c452ae");
            vSeeds.emplace_back("178.128.221.177");
            vSeeds.emplace_back("2a01:7c8:d005:390::5");
            vSeeds.emplace_back("ntv3mtqw5wt63red.onion:38333");

            consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000000000000000019fd16269a");
            consensus.defaultAssumeValid = uint256S("0x0000002a1de0f46379358c1fd09906f7ac59adf3712323ed90eb59e4c183c020"); // 9434
            m_assumed_blockchain_size = 1;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{
                // Data from RPC: getchaintxstats 4096 0000002a1de0f46379358c1fd09906f7ac59adf3712323ed90eb59e4c183c020
                /* nTime    */ 1603986000,
                /* nTxCount */ 9582,
                /* dTxRate  */ 0.00159272030651341,
            };
        } else {
            const auto signet_challenge = args.GetArgs("-signetchallenge");
            if (signet_challenge.size() != 1) {
                throw std::runtime_error(strprintf("%s: -signetchallenge cannot be multiple values.", __func__));
            }
            bin = ParseHex(signet_challenge[0]);

            consensus.nMinimumChainWork = uint256{};
            consensus.defaultAssumeValid = uint256{};
            m_assumed_blockchain_size = 0;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{
                0,
                0,
                0,
            };
            LogPrintf("Signet with challenge %s\n", signet_challenge[0]);
        }

        if (args.IsArgSet("-signetseednode")) {
            vSeeds = args.GetArgs("-signetseednode");
        }

        strNetworkID = CBaseChainParams::SIGNET;
        consensus.signet_blocks = true;
        consensus.signet_challenge.assign(bin.begin(), bin.end());
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP16Exception = uint256{};
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 1;
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Activation of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        consensus.PoSRewardCoins = 50;

        consensus.rewardExtraAddresses = {
            {"UgD6oyq8Rxn66D81t28TBfvxXsQGCp1jSn", 1},      // Team
            {"UVuXQH5raXQK7RL2nkgXDtGTfEtCQNc8y5", 1},     // Community
        };

        consensus.littleStakerPercentage = 0.2;
        consensus.littleStakerMinimumCoins = 100 * COIN;

        consensus.mediumStakerPercentage = 0.4;
        consensus.mediumStakerMinimumCoins = 250 * COIN;

        consensus.bigStakerPercentage = 0.9;
        consensus.bigStakerMinimumCoins = 1000 * COIN;

        consensus.minimumStakerPercentage = 0.05; // 5 %

        consensus.plywoodRewardAddress = "abc"; // AirDrop & Marketing
        consensus.AirDropRewardAddress = ""; // AirDrop

        // message start is defined as the first 4 bytes of the sha256d of the block script
        CHashWriter h(SER_DISK, 0);
        h << consensus.signet_challenge;
        uint256 hash = h.GetHash();
        memcpy(pchMessageStart, hash.begin(), 4);

        nDefaultPort = 38333;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1598918400, 52613770, 0x1e0377ae, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        uint32_t nB = 0x1e0fffff;
        //mineNow("[SigNet]", consensus, genesis, nB);
        // BPSTODO - generate these
        ////assert(consensus.hashGenesisBlock == uint256S("0x00000008819873e925422c1ff0f99f7cc9bbb232af63a077a480a3633bee1ef6"));
        ////assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        vFixedSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "nls";

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = true;
        m_is_mockable_chain = false;

        NormalizeRewards(consensus);
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    explicit CRegTestParams(const ArgsManager& args) {
        strNetworkID =  CBaseChainParams::REGTEST;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP16Exception = uint256();
        consensus.BIP34Height = 500; // BIP34 activated on regtest (Used in functional tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in functional tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in functional tests)
        consensus.CSVHeight = 432; // CSV activated on regtest (Used in rpc activation tests)
        consensus.SegwitHeight = 0; // SEGWIT is always activated on regtest unless overridden
        consensus.MinBIP9WarningHeight = 0;
        consensus.BPSRewardMatchStep = 400;
        consensus.BPSRewardMatchHeight = 3 * consensus.BPSRewardMatchStep;
        consensus.BPSDiffAdjHeight = 1500;
        consensus.EnableStackingAtBlock = 2000;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 10 * 3 * 60; // every 10 blocks
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.fPoSNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.nMPoSRewardRecipients = 10;

        consensus.PoSRewardCoins = 50;

        consensus.rewardExtraAddresses = {
            {"UgD6oyq8Rxn66D81t28TBfvxXsQGCp1jSn", 1},      // Team
            {"UVuXQH5raXQK7RL2nkgXDtGTfEtCQNc8y5", 1},     // Community
        };

        consensus.littleStakerPercentage = 0.2;
        consensus.littleStakerMinimumCoins = 100 * COIN;

        consensus.mediumStakerPercentage = 0.4;
        consensus.mediumStakerMinimumCoins = 250 * COIN;

        consensus.bigStakerPercentage = 0.9;
        consensus.bigStakerMinimumCoins = 1000 * COIN;

        consensus.minimumStakerPercentage = 0.05; // 5 %

        consensus.plywoodRewardAddress = "abc"; // AirDrop & Marketing
        consensus.AirDropRewardAddress = ""; // AirDrop
        
        consensus.nEnableHeaderSignatureHeight = 0;
        consensus.nCheckpointSpan = COINBASE_MATURITY;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        consensus.nMinimumChainWork = uint256{};
        consensus.defaultAssumeValid = uint256{};

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 48934;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        UpdateActivationParametersFromArgs(args);

        genesis = CreateGenesisBlock(1588417200, 0, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        uint32_t nB = 0x1e0fffff;
        //mineNow("[RegTest]", consensus, genesis, nB);
        //assert(consensus.hashGenesisBlock == uint256S("0x2b8d445931aa4ea9b52db1488d3641fa2d4f7a3c1f8151bfa99d017493129e97"));
        //assert(genesis.hashMerkleRoot == uint256S("0xb44e2d41890cc021a91405d7944b77ac4a27fadfc0caa9734c68fc64da09a207"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = true;
        m_is_test_chain = true;
        m_is_mockable_chain = true;

        checkpointData = {
            {
                //{0, uint256S("0x2b8d445931aa4ea9b52db1488d3641fa2d4f7a3c1f8151bfa99d017493129e97")},
                { 0, consensus.hashGenesisBlock },
            }
        };

        chainTxData = ChainTxData{
            1588417200,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,65);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,78);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,130);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "ncr";

        NormalizeRewards(consensus);
    }

    /**
     * Allows modifying the Version Bits regtest parameters.
     */
    void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
    void UpdateActivationParametersFromArgs(const ArgsManager& args);
};

void CRegTestParams::UpdateActivationParametersFromArgs(const ArgsManager& args)
{
    if (args.IsArgSet("-segwitheight")) {
        int64_t height = args.GetArg("-segwitheight", consensus.SegwitHeight);
        if (height < -1 || height >= std::numeric_limits<int>::max()) {
            throw std::runtime_error(strprintf("Activation height %ld for segwit is out of valid range. Use -1 to disable segwit.", height));
        } else if (height == -1) {
            LogPrintf("Segwit disabled for testing\n");
            height = std::numeric_limits<int>::max();
        }
        consensus.SegwitHeight = static_cast<int>(height);
    }

    if (!args.IsArgSet("-vbparams")) return;

    for (const std::string& strDeployment : args.GetArgs("-vbparams")) {
        std::vector<std::string> vDeploymentParams;
        boost::split(vDeploymentParams, strDeployment, boost::is_any_of(":"));
        if (vDeploymentParams.size() != 3) {
            throw std::runtime_error("Version bits parameters malformed, expecting deployment:start:end");
        }
        int64_t nStartTime, nTimeout;
        if (!ParseInt64(vDeploymentParams[1], &nStartTime)) {
            throw std::runtime_error(strprintf("Invalid nStartTime (%s)", vDeploymentParams[1]));
        }
        if (!ParseInt64(vDeploymentParams[2], &nTimeout)) {
            throw std::runtime_error(strprintf("Invalid nTimeout (%s)", vDeploymentParams[2]));
        }
        bool found = false;
        for (int j=0; j < (int)Consensus::MAX_VERSION_BITS_DEPLOYMENTS; ++j) {
            if (vDeploymentParams[0] == VersionBitsDeploymentInfo[j].name) {
                UpdateVersionBitsParameters(Consensus::DeploymentPos(j), nStartTime, nTimeout);
                found = true;
                LogPrintf("Setting version bits activation parameters for %s to start=%ld, timeout=%ld\n", vDeploymentParams[0], nStartTime, nTimeout);
                break;
            }
        }
        if (!found) {
            throw std::runtime_error(strprintf("Invalid deployment (%s)", vDeploymentParams[0]));
        }
    }
}

static std::unique_ptr<const CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<const CChainParams> CreateChainParams(const ArgsManager& args, const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN) {
        return std::unique_ptr<CChainParams>(new CMainParams());
    } else if (chain == CBaseChainParams::TESTNET) {
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    } else if (chain == CBaseChainParams::SIGNET) {
        return std::unique_ptr<CChainParams>(new SigNetParams(args));
    } else if (chain == CBaseChainParams::REGTEST) {
        return std::unique_ptr<CChainParams>(new CRegTestParams(args));
    }
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(gArgs, network);
}
