from setuptools import Extension, setup

module = Extension("asdf", sources=['symnmf.c','symnmfmodule.c'])
setup(name='asdf',
      version='1.0',
      description='Python wrapper for custom C extension',
      ext_modules=[module])