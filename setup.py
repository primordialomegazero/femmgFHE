from setuptools import setup, find_packages

setup(
    name="femmgfhe",
    version="7.0.0",
    description="FEmmg-FHE: Statistical Noise Cancellation for Fully Homomorphic Encryption",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    author="Dan Joseph M. Fernandez / Primordial Omega Zero",
    author_email="primordialomegazero@github",
    url="https://github.com/primordialomegazero/femmgFHE",
    packages=find_packages(),
    python_requires=">=3.8",
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Topic :: Security :: Cryptography",
    ],
)
