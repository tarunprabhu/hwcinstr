// Copyright 2020 Tarun Prabhu
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "CFEContext.h"

#include <openssl/md5.h>

#include <cstdio>
#include <sstream>

using llvm::cast;
using llvm::cast_or_null;

// Singleton context object that will be used by both the Clang plugin and
// the LLVM pass. The only interface to this will be through the functions
// defined here
//
static CFEContext* gCFEContext;

CFEContext& CFEContext::getSingleton() {
  if(not gCFEContext)
    gCFEContext = new CFEContext;
  return *gCFEContext;
}

CFEContext::CFEContext() : papiContext(true), conf(papiContext) {
  ;
}

template <typename IDType>
static IDType constructID(std::string s) {
  const unsigned char* digest
      = reinterpret_cast<const unsigned char*>(s.c_str());
  unsigned char md5[MD5_DIGEST_LENGTH];

  MD5(digest, s.length(), md5);

  // Read the 8 bytes of the 16 byte digest. Start somewhere in the middle for no
  // good reason
  IDType id = 0;
  for(unsigned start = 3, i = start; i < start + 8; i++) {
    id <<= 8;
    id |= md5[i];
  }

  return id;
}

static RegionID constructRegionID(const std::string& file,
                                  unsigned startLine,
                                  unsigned endLine,
                                  const std::vector<CounterID>& counters) {
  // This is a ridiculous way of getting a hash

  // Concatenate everything including the counter ids. They are added to
  // minimize the small chances of a collision
  std::stringstream ss;
  ss << file << ":" << startLine << ":" << endLine;
  for(CounterID id : counters)
    ss << ":" << id;

  return constructID<RegionID>(ss.str());
}

static FunctionID constructFunctionID(const std::string& mangled) {
  return constructID<FunctionID>(mangled);
}

void CFEContext::addFunction(const std::string& mangled,
                             const std::string& srcName,
                             const std::string& qualName,
                             const std::vector<CounterID>& counters) {
  funcs.emplace(std::pair<std::string, hwc::FEFuncMeta>(
      mangled,
      {constructFunctionID(mangled),
       counters,
       srcName,
       (srcName != qualName) ? qualName : ""}));
}

void CFEContext::addRegion(const std::string& file,
                           unsigned startLine,
                           unsigned endLine,
                           const std::vector<CounterID>& counters) {
  regions.emplace_back(constructRegionID(file, startLine, endLine, counters),
                       counters,
                       file,
                       startLine,
                       endLine);
}

bool CFEContext::shouldInstrument(llvm::Function& f) const {
  return funcs.find(f.getName()) != funcs.end();
}

const hwc::FEFuncMeta& CFEContext::getFuncMeta(llvm::Function& f) const {
  return funcs.at(f.getName());
}

CFEContext::region_range CFEContext::getRegions() const {
  return region_range(regions.begin(), regions.end());
}

const PAPIContext& CFEContext::getPAPIContext() const {
  return papiContext;
}

Conf& CFEContext::getConf() {
  return conf;
}

const Conf& CFEContext::getConf() const {
  return conf;
}
