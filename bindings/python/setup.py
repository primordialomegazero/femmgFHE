from setuptools import setup, find_packages

setup(
    name="femmgfhe",
    version="0.1.0",
    description="FEmmg-FHE Python Bindings — Fully Homomorphic Encryption",
    author="Dan Joseph M. Fernandez",
    author_email="djmfernandez@proton.me",
    packages=find_packages(),
    python_requires=">=3.8",
    install_requires=["cffi>=1.15.0"],
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Topic :: Security :: Cryptography",
        "Topic :: Scientific/Engineering :: Mathematics",
    ],
)
