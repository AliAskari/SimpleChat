## Generate documentation with Doxygen and Sphinx

This tool generate API references from code that doxygen supported.

This tool is based on Sphinx with Breathe and Doxygen. So you can treat it as a combination of Doxygen and Sphinx.

### Install

#### Requirements
  * Doxygen
  * Sphinx
  * Breathe
  * sphinx_rtd_theme
  * recommonmark

Run the command to install dependences
```
pip install -r requirements.txt
```

### Usage
1. Put the rep into a folder of your project.
2. Edit the `doc.json` file to setup your project.
3. Run `python gendoc.py` to genrate documentations with doxygen.
4. Write other documentations with reStructruedText or Markdown with Sphinx workflow.
5. Run `python gendoc.py html` to generate documentation with Sphinx
