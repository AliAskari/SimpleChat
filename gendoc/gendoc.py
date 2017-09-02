#!/usr/bin/python
# -*- coding: utf-8 -*-
import fnmatch
import os
import sys
import shutil
import json
from breathe import parser

helpContent = """\
Generate documentations from your C/C++ code with Doxygen and Sphinx
Usage:

    python gendoc.py
    Setup sphinx project for documentation

    python gendoc.py [option]
    Options:
        -h/--help :  Show this help information
        clean:    :  Remove all files generated
        html      :  Generate html documents with Sphix
        pdf       :  Generate html documents with Sphix
        epub      :  Generate epub documents with Sphix
        qthelp    :  Generate documents for QtCreator with Sphix
"""


def globPath(path, pattern):
    result = []
    for root, subdirs, files in os.walk(path):
        for filename in files:
            if fnmatch.fnmatch(filename, pattern):
                result.append(os.path.join(root, filename))
    return result


def rm(path):
    if os.path.exists(path):
        if os.path.isdir(path):
            shutil.rmtree(path)
        else:
            os.remove(path)


def runDoxygen(meta):
    print("========== Generating doxygen xml ============")
    doxyxmldir = os.path.join(meta["build_dir"], "doxygen", "xml")
    if not os.path.isdir(doxyxmldir):
        os.makedirs(doxyxmldir)
    content = open("_templates{}doxyfile".format(os.sep)).read()
    for k in meta:
        if isinstance(meta[k], unicode) or isinstance(meta[k], str):
            content = content.replace("${" + k + "}", meta[k])
    doxyfile = os.path.join(doxyxmldir, "..", "doxyfile")
    f = open(doxyfile, "w")
    f.write(content)
    f.close()
    os.system("doxygen {}".format(doxyfile))


def parseDoxygen():
    print("========== Parsing doxygen ============")
    index = None
    compounds = []
    doxyxmldir = os.path.join(meta["build_dir"], "doxygen", "xml")
    for xml in globPath(doxyxmldir, "*.xml"):
        print("parsing {}".format(xml))
        if xml.endswith("index.xml"):
            index = parser.index.parse(xml)
        else:
            try:
                compounds.append(parser.compound.parse(xml))
            except Exception:
                print("Error parsing: {}".format(xml))

    symbols = {}
    for c in compounds:
        for section in c.compounddef.sectiondef:
            kind = section.kind
            members = []
            for member in section.memberdef:
                members.append(member)
            if kind not in symbols:
                symbols[kind] = members
            else:
                symbols[kind] += members
    print("finished...")
    return index, symbols


def genClasses(index, symbols, meta):
    print("========== Generating classes ============")
    classtmp = open("_templates/class.rst").read()
    classes = []
    for c in index.compound:
        if c.kind in ["class", "struct"]:
            classes.append((c.name, c.kind))
    classes.sort()
    for c in classes:
        name = c[0]
        type = c[1]
        if name in meta["ignore"]["classes"] or "<" in name:
            continue
        print(name)
        classdir = os.path.join(meta["api_dir"], "classes")
        if not os.path.isdir(classdir):
            os.makedirs(classdir)
        classrst = classtmp.replace("${class}", name)
        classrst = classrst.replace("${type}", type)
        filename = os.path.join(classdir, name.replace("::", "_") + ".rst")
        classrstf = open(filename, "w")
        classrstf.write(classrst)
        classrstf.close()
    shutil.copyfile("_templates{}classes.rst".format(os.sep),
                    os.path.join(meta["api_dir"], "_classes.rst"))
    print("finished...")


def genSymbols(index, doxySymbols, meta):
    print("========== Generating other symbols ============")
    resovedCompounds = ["namespace", "group", "dir", "file", "module"]
    resovedMembers = ["define", "enum",
                      "function", "typedef", "variable", "union"]
    aliasMap = {
        "function": "func",
        "variable": "var",
        "enumvalue": "enum",
    }
    symbols = {}
    for c in index.compound:
        kind = c.kind
        if kind in resovedCompounds:
            for m in c.member:
                k = m.kind
                if not symbols.has_key(k):
                    symbols[k] = []
                ak = k
                if k in aliasMap:
                    ak = aliasMap[k]
                rawmember = None
                for rm in doxySymbols[ak]:
                    if rm.id == m.refid:
                        rawmember = rm
                        break
                symbols[k].append((m.name, m, rawmember))

    for k in symbols:
        if meta["with_symbols"].has_key(k) and not meta["with_symbols"][k]:
            continue
        if len(symbols[k]) == 0:
            continue
        print(k)
        symbols[k].sort()
        rstContent = ""
        rstContent += "{}\r\n".format(k.capitalize())
        rstContent += "=" * len(k) + "\r\n"
        for pair in symbols[k]:
            name = pair[0]
            m = pair[1]
            rawm = pair[2]
            if k == "function" and rawm is not None:
                name = rawm.definition.split(" ")[-1]
                name += rawm.argsstring
                if "set" in name:
                    print m
            if name in meta["ignore"]["symbols"]:
                continue
            rstContent += ".. doxygen{}:: {}\r\n".format(k, name)

        rstContent += "\r\n"
        rst = open(os.path.join(meta["api_dir"], "_" + k + ".rst"), "w")
        rst.write(rstContent)
        rst.close()
    print("finished...")


def setupSphinx(meta):
    print("========== Setup Sphinx ============")

    def replaceContent(tmplt, target):
        content = open(tmplt).read()
        for k in meta:
            if isinstance(meta[k], unicode) or isinstance(meta[k], str):
                content = content.replace("${" + k + "}", meta[k])
        f = open(target, "w")
        f.write(content)
        f.close()
        print(target)
    replaceContent("_templates/conf.py", "conf.py")
    replaceContent("_templates/make.bat", "make.bat")
    replaceContent("_templates/Makefile", "Makefile")
    print("finished...")


def actionClean(meta):
    print("========== Clean generated files ============")

    def rmpath(path):
        rm(path)
        print(path)
    rmpath(meta["api_dir"])
    rmpath(meta["build_dir"])
    rmpath("conf.py")
    rmpath("make.bat")
    rmpath("Makefile")
    print("finished...")


if __name__ == '__main__':
    meta = json.load(open("doc.json"))
    if len(sys.argv) > 1:
        param = sys.argv[1]
        if param == "clean":
            actionClean(meta)
        elif param in ["-h", "--help"]:
            print(helpContent)
        else:
            os.system("make {}".format(param))
    else:
        runDoxygen(meta)
        index, symbols = parseDoxygen()

        genClasses(index, symbols, meta)
        genSymbols(index, symbols, meta)
        setupSphinx(meta)
