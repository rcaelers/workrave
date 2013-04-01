#!/usr/bin/python3
#
# Copyright (C) Maarten Bosmans 2011, 2012
#
# The contents of this file are subject to the Mozilla Public License Version 1.1; you may not use this file except in
# compliance with the License. You may obtain a copy of the License at http://www.mozilla.org/MPL/
#
from urllib.request import urlretrieve, urlopen
from logging import warning, error
import logging
import os.path
import sys

_packages = []

_scriptDirectory = os.path.dirname(os.path.realpath(__file__))
_packageCacheDirectory = os.path.join(_scriptDirectory, 'cache', 'package')
_repositoryCacheDirectory = os.path.join(_scriptDirectory, 'cache', 'repository')
_extractedCacheDirectory = os.path.join(_scriptDirectory, 'cache', 'extracted')
_extractedFilesDirectory = _scriptDirectory


def OpenRepository(repositoryLocation):
  from xml.etree.cElementTree import parse as xmlparse
  global _packages
  # Check repository for latest primary.xml
  with urlopen(repositoryLocation + 'repodata/repomd.xml') as metadata:
    doctree = xmlparse(metadata)
  xmlns = 'http://linux.duke.edu/metadata/repo'
  for element in doctree.findall('{%s}data'%xmlns):
    if element.get('type') == 'primary':
      primaryUrl = element.find('{%s}location'%xmlns).get('href')
  # Make sure all the cache directories exist
  for dir in _packageCacheDirectory, _repositoryCacheDirectory, _extractedCacheDirectory:
    try:
      os.makedirs(dir)
    except OSError: pass
  # Download repository metadata (only if not already in cache)
  primaryFilename = os.path.join(_repositoryCacheDirectory, os.path.splitext(os.path.basename(primaryUrl))[0])
  if not os.path.exists(primaryFilename):
    warning('Dowloading repository data')
    with urlopen(repositoryLocation + primaryUrl) as primaryGzFile:
      import io, gzip
      primaryGzString = io.BytesIO(primaryGzFile.read()) #3.2: use gzip.decompress
      with gzip.GzipFile(fileobj=primaryGzString) as primaryGzipFile:
        with open(primaryFilename, 'wb') as primaryFile:
          primaryFile.writelines(primaryGzipFile)
  elements = xmlparse(primaryFilename)
  # Parse package list from XML
  xmlns = 'http://linux.duke.edu/metadata/common'
  rpmns = 'http://linux.duke.edu/metadata/rpm'
  _packages = [{
      'name': p.find('{%s}name'%xmlns).text,
      'arch': p.find('{%s}arch'%xmlns).text,
      'buildtime': int(p.find('{%s}time'%xmlns).get('build')),
      'url': repositoryLocation + p.find('{%s}location'%xmlns).get('href'),
      'filename': os.path.basename(p.find('{%s}location'%xmlns).get('href')),
      'provides': {provides.attrib['name'] for provides in p.findall('{%s}format/{%s}provides/{%s}entry'%(xmlns,rpmns,rpmns))},
      'requires': {req.attrib['name'] for req in p.findall('{%s}format/{%s}requires/{%s}entry'%(xmlns,rpmns,rpmns))}
    } for p in elements.findall('{%s}package'%xmlns)]


def _findPackage(packageName, srcpkg=False):
  filter_func = lambda p: (p['name'] == packageName or p['filename'] == packageName) and p['arch'] == ('src' if srcpkg else 'noarch')
  sort_func = lambda p: p['buildtime']
  packages = sorted([p for p in _packages if filter_func(p)], key=sort_func, reverse=True)
  if len(packages) == 0:
    return None
  if len(packages) > 1:
    error('multiple packages found for %s:', packageName)
    for p in packages:
      error('  %s', p['filename'])
  return packages[0]


def _checkPackageRequirements(package, packageNames):
  allProviders = set()
  for requirement in package['requires']:
    providers = {p['name'] for p in _packages if requirement in p['provides']}
    if len(providers & packageNames) == 0:
      if len(providers) == 0:
        error('Package %s requires %s, not provided by any package', package['name'], requirement)
      else:
        warning('Package %s requires %s, provided by: %s', package['name'], requirement, ','.join(providers))
        allProviders.add(providers.pop())
  return allProviders


def packagesDownload(packageNames, withDependencies=False, srcpkg=False):
  from fnmatch import fnmatchcase
  packageNames_new = {pn for pn in packageNames if pn.endswith('.rpm')}
  for packageName in packageNames - packageNames_new:
    matchedpackages = {p['name'] for p in _packages if fnmatchcase(p['name'].replace('mingw32-', '').replace('mingw64-', ''), packageName) and p['arch'] == ('src' if srcpkg else 'noarch')}
    packageNames_new |= matchedpackages if len(matchedpackages) > 0 else {packageName}
  packageNames = list(packageNames_new)
  allPackageNames = set(packageNames)

  packageFilenames = []
  while len(packageNames) > 0:
    packName = packageNames.pop()
    package = _findPackage(packName, srcpkg)
    if package == None:
      error('Package %s not found', packName)
      continue
    dependencies = _checkPackageRequirements(package, allPackageNames)
    if withDependencies and len(dependencies) > 0:
      packageNames.extend(dependencies)
      allPackageNames |= dependencies
    localFilenameFull = os.path.join(_packageCacheDirectory, package['filename'])
    if not os.path.exists(localFilenameFull):
      warning('Downloading %s', package['filename'])
      urlretrieve(package['url'], localFilenameFull)
    packageFilenames.append(package['filename'])
  return packageFilenames


def _extractFile(filename, output_dir=_extractedCacheDirectory):
  from subprocess import check_call
  try:
    with open('7z.log', 'w') as logfile:
      check_call(['7z', 'x', '-o'+output_dir, '-y', filename], stdout=logfile)
    os.remove('7z.log')
  except:
    error('Failed to extract %s', filename)


def packagesExtract(packageFilenames, srcpkg=False):
  for packageFilename in packageFilenames :
    warning('Extracting %s', packageFilename)
    cpioFilename = os.path.join(_extractedCacheDirectory, os.path.splitext(packageFilename)[0] + '.cpio')
    if not os.path.exists(cpioFilename):
      _extractFile(os.path.join(_packageCacheDirectory, packageFilename))
    if srcpkg:
      _extractFile(cpioFilename, os.path.join(_extractedFilesDirectory, os.path.splitext(packageFilename)[0]))
    else:
      _extractFile(cpioFilename, _extractedFilesDirectory)


def GetBaseDirectory():
  if os.path.exists(os.path.join(_extractedFilesDirectory, 'usr/i686-w64-mingw32/sys-root/mingw')):
    return os.path.join(_extractedFilesDirectory, 'usr/i686-w64-mingw32/sys-root/mingw')
  if os.path.exists(os.path.join(_extractedFilesDirectory, 'usr/x86_64-w64-mingw32/sys-root/mingw')):
    return os.path.join(_extractedFilesDirectory, 'usr/x86_64-w64-mingw32/sys-root/mingw')
  return _extractedFilesDirectory


def CleanExtracted():
  from shutil import rmtree
  rmtree(os.path.join(_extractedFilesDirectory, 'usr'), True)


def SetExecutableBit():
  # set executable bit on libraries and executables
  for root, dirs, files in os.walk(GetBaseDirectory()):
    for filename in {f for f in files if f.endswith('.dll') or f.endswith('.exe')} | set(dirs):
      os.chmod(os.path.join(root, filename), 0o755)


def GetOptions():
  from optparse import OptionParser, OptionGroup #3.2: use argparse

  parser = OptionParser(usage="usage: %prog [options] packages",
                        description="Easy download of RPM packages for Windows.")

  # Options specifiying download repository
  default_project = "windows:mingw:win32"
  default_repository = "openSUSE_12.3"
  default_repo_url = "http://download.opensuse.org/repositories/PROJECT/REPOSITORY/"
  repoOptions = OptionGroup(parser, "Specify download repository")
  repoOptions.add_option("-p", "--project", dest="project", default=default_project,
                         metavar="PROJECT", help="Download from PROJECT [%default]")
  repoOptions.add_option("-r", "--repository", dest="repository", default=default_repository,
                         metavar="REPOSITORY", help="Download from REPOSITORY [%default]")
  repoOptions.add_option("-u", "--repo-url", dest="repo_url", default=default_repo_url,
                         metavar="URL", help="Download packages from URL (overrides PROJECT and REPOSITORY options) [%default]")
  parser.add_option_group(repoOptions)

  # Package selection options
  parser.set_defaults(withdeps=False)
  packageOptions = OptionGroup(parser, "Package selection")
  packageOptions.add_option("--deps", action="store_true", dest="withdeps", help="Download dependencies")
  packageOptions.add_option("--no-deps", action="store_false", dest="withdeps", help="Do not download dependencies [default]")
  packageOptions.add_option("--src", action="store_true", dest="srcpkg", default=False, help="Download source instead of noarch package")
  parser.add_option_group(packageOptions)

  # Output options
  outputOptions = OptionGroup(parser, "Output options", "Normally the downloaded packages are extracted in the current directory.")
  outputOptions.add_option("--no-clean", action="store_false", dest="clean", default=True,
                           help="Do not remove previously extracted files")
  outputOptions.add_option("-z", "--make-zip", action="store_true", dest="makezip", default=False,
                           help="Make a zip file of the extracted packages (the name of the zip file is based on the first package specified)")
  outputOptions.add_option("-m", "--add-metadata", action="store_true", dest="metadata", default=False,
                           help="Add a file containing package dependencies and provides")
  parser.add_option_group(outputOptions)

  # Other options
  parser.add_option("-q", "--quiet", action="store_false", dest="verbose", default=True,
                    help="Don't print status messages to stderr")

  (options, args) = parser.parse_args()

  if len(args) == 0:
    parser.print_help(file=sys.stderr)
    sys.exit(1)

  return (options, args)


def main():
  import re, zipfile 

  (options, args) = GetOptions()
  packages = set(args)
  logging.basicConfig(level=(logging.WARNING if options.verbose else logging.ERROR), format='%(message)s', stream=sys.stderr)

  # Open repository
  repository = options.repo_url.replace("PROJECT", options.project.replace(':', ':/')).replace("REPOSITORY", options.repository)
  try:
    OpenRepository(repository)
  except Exception as e:
    sys.exit('Error opening repository:\n\t%s\n\t%s' % (repository, e))

  if options.clean:
    CleanExtracted()

  if options.makezip or options.metadata:
    package = _findPackage(args[0]) #if args[0].endswith('.rpm') 
    if package == None: package = _findPackage("mingw32-"+args[0], options.srcpkg)
    if package == None: package = _findPackage("mingw64-"+args[0], options.srcpkg)
    if package == None:
      sys.exit('Package not found:\n\t%s' % args[0])
    packageBasename = re.sub('^mingw(32|64)-|\\.noarch|\\.rpm$', '', package['filename'])

  packages = packagesDownload(packages, options.withdeps, options.srcpkg)
  for package in sorted(packages):
    print(package)

  packagesExtract(packages, options.srcpkg)
  SetExecutableBit()

  if options.metadata:
    cleanup = lambda n: re.sub('^mingw(?:32|64)-(.*)', '\\1', re.sub('^mingw(?:32|64)[(](.*)[)]', '\\1', n))
    with open(os.path.join(GetBaseDirectory(), packageBasename + '.metadata'), 'w') as m:
      for packageFilename in sorted(packages):
        package = [p for p in _packages if p['filename'] == packageFilename][0]
        m.writelines(['provides:%s\r\n' % cleanup(p) for p in package['provides']])
        m.writelines(['requires:%s\r\n' % cleanup(r) for r in package['requires']])

  if options.makezip:
    packagezip = zipfile.ZipFile(packageBasename + '.zip', 'w', compression=zipfile.ZIP_DEFLATED)
    for root, dirs, files in os.walk(GetBaseDirectory()):
      for filename in files:
        fullname = os.path.join(root, filename)
        packagezip.write(fullname, fullname.replace(GetBaseDirectory(), ''))
    packagezip.close() #3.2: use with
    if options.clean:
      CleanExtracted()

if __name__ == "__main__":
    main()

