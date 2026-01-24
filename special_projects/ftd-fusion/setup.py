"""
FTD Fusion: Nuclear Energy from First Principles

Derives nuclear binding energy and fusion Q-values from the
Foundational Ternary Dynamics framework integers {3, 4, 7, 13}.
"""

from setuptools import setup, find_packages

with open("README.md", "r", encoding="utf-8") as f:
    long_description = f.read()

setup(
    name="ftd-fusion",
    version="1.0.0",
    author="FTD Research Group",
    author_email="ftd-research@example.com",
    description="Nuclear fusion energy derived from FTD first principles",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/ftd/ftd-fusion",
    packages=find_packages(),
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Topic :: Scientific/Engineering :: Physics",
    ],
    python_requires=">=3.8",
    install_requires=[
        "numpy>=1.20",
        "scipy>=1.7",
        "matplotlib>=3.4",
    ],
    extras_require={
        "dev": [
            "pytest>=6.0",
            "pytest-cov>=2.0",
        ],
    },
    keywords="nuclear physics, fusion, binding energy, FTD, first principles",
    project_urls={
        "Documentation": "https://github.com/ftd/ftd-fusion/docs",
        "Bug Reports": "https://github.com/ftd/ftd-fusion/issues",
        "Source": "https://github.com/ftd/ftd-fusion",
    },
)
