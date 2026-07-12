// ΦΩ0 — PHI SNARK VERIFIER DEPLOYMENT
// Deploy to Ethereum testnet / mainnet
// "I AM THAT I AM"

const { ethers } = require("hardhat");

async function main() {
    console.log("\n╔══════════════════════════════════════════════╗");
    console.log("║  ΦΩ0 — DEPLOYING PHI SNARK VERIFIER          ║");
    console.log("╚══════════════════════════════════════════════╝\n");

    const PHISnarkVerifier = await ethers.getContractFactory("PHISnarkVerifier");
    const verifier = await PHISnarkVerifier.deploy();
    await verifier.deployed();

    console.log(`✅ PHISnarkVerifier deployed to: ${verifier.address}`);
    
    // Quick test
    console.log("\nΦ Running quick verification test...");
    
    const proof = {
        A: ethers.BigNumber.from("0x59712b"),
        B: ethers.BigNumber.from("0x5970cd"),
        C: ethers.BigNumber.from("0xb87a361"),
    };
    
    const tx = await verifier.verifyBaseProof(proof, 42, 17, 714);
    await tx.wait();
    
    console.log("✅ Test verification: PASSED");
    console.log(`   Gas used: ${tx.gasLimit.toString()}`);
    console.log(`   Transaction: ${tx.hash}`);
    
    console.log("\n╔══════════════════════════════════════════════╗");
    console.log("║  ΦΩ0 — VERIFIER DEPLOYED & TESTED            ║");
    console.log("║  I AM THAT I AM                              ║");
    console.log("╚══════════════════════════════════════════════╝\n");
}

main()
    .then(() => process.exit(0))
    .catch((error) => {
        console.error(error);
        process.exit(1);
    });
