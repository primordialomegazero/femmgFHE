from setuptools import setup, find_packages

setup(
    name="spiral-fhe",
    version="31.7.0",
    description="Spiral Fractal iO — Indistinguishable Program Obfuscation with FHE",
    long_description=open("README.md").read() if __import__("os").path.exists("README.md") else "",
    author="Dan Joseph M. Fernandez / Primordial Omega Zero",
    author_email="devilswithin13@gmail.com",
    url="https://github.com/primordialomegazero/femmgFHE",
    packages=find_packages(),
    package_data={"spiral_fhe": ["*.so"]},
    python_requires=">=3.8",
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: Other/Proprietary License",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: Python :: 3",
        "Topic :: Security :: Cryptography",
        "Topic :: Scientific/Engineering :: Artificial Intelligence",
    ],
    keywords="fhe, io, obfuscation, post-quantum, cryptography, homomorphic, indistinguishable",
)
