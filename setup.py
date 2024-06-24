from setuptools import setup, find_packages  # Import setuptools
import pathlib

NAME = "eefpy"
URL = "https://github.com/ariel-research/" + NAME
HERE = pathlib.Path(__file__).parent
README = (HERE / "README.md").read_text()
REQUIRES = (HERE / "requirements.txt").read_text().strip().split("\n")
REQUIRES = [lin.strip() for lin in REQUIRES]

setup(
    name=NAME,
    description='python eef practical solver', 
    long_description=README,
    long_description_content_type='text/markdown',
    url=URL,
    license='GNU',
    packages=find_packages(),
    install_requires=REQUIRES,
)
