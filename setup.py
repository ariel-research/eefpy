from setuptools import setup, find_packages  # Import setuptools
import pathlib

NAME = "eefpy"
URL = "https://github.com/ariel-research/" + NAME
HERE = pathlib.Path(__file__).parent
VERSION = (HERE / NAME / "VERSION").read_text().strip()

setup(
    name='eefpy',
    version=VERSION,
    description='python eef practical solver', 
    long_description=open('README.md').read(),
    long_description_content_type='text/markdown',
    url='https://github.com/ariel-research/eefpy',
    include_package_data=True,
    license='GNU',
    packages=find_packages(),
    install_requires=[
        'cppyy',
    ],
)
