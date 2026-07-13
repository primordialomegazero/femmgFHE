from setuptools import setup, Extension
import pybind11

ext = Extension(
    'femmg_fhe',
    sources=['femmg_fhe_bindings.cpp'],
    include_dirs=[
        pybind11.get_include(),
        '/usr/local/include/openfhe',
        '/usr/local/include/openfhe/core',
        '/usr/local/include/openfhe/pke',
        '/usr/local/include/openfhe/binfhe',
    ],
    library_dirs=['/usr/local/lib'],
    libraries=['OPENFHEcore', 'OPENFHEpke', 'OPENFHEbinfhe', 'ssl', 'crypto', 'pthread'],
    runtime_library_dirs=['/usr/local/lib'],
    extra_compile_args=[
        '-std=c++17', 
        '-O2', 
        '-fPIC',
        '-Wno-unknown-pragmas',      # Suppress OpenMP pragma warnings
        '-Wno-unused-variable',      # Suppress unused variable warnings
        '-Wno-sign-compare',         # Suppress sign comparison warnings
    ],
    extra_link_args=[
        '-Wl,--enable-new-dtags',
        '-Wl,-rpath,/usr/local/lib',
    ],
)

setup(
    name='femmg_fhe',
    version='5.0.0',
    description='ΦΩ0 — FEmmg-FHE: Fully Homomorphic Encryption with ZANS',
    ext_modules=[ext],
    install_requires=['pybind11>=2.10.0'],
)
