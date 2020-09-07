/*
 * Copyright (C) 2008 Tommi Maekitalo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include <zim/fileheader.h>
#include <zim/error.h>
#include <iostream>
#include <algorithm>
#include "log.h"
#include "endian_tools.h"
#include "idatastream.h"
#ifdef _WIN32
# include "io.h"
#else
# include "unistd.h"
# define _write(fd, addr, size) if(::write((fd), (addr), (size)) != (ssize_t)(size)) \
{throw std::runtime_error("Error writing");}
#endif

log_define("zim.file.header")

namespace zim
{
  const uint32_t Fileheader::zimMagic = 0x044d495a; // ="ZIM^d"
  const uint16_t Fileheader::zimClassicMajorVersion = 5;
  const uint16_t Fileheader::zimExtendedMajorVersion = 6;
  const uint16_t Fileheader::zimMinorVersion = 0;
  const offset_type Fileheader::size = 80; // This is also mimeListPos (so an offset)

  void Fileheader::write(int out_fd) const
  {
    char header[Fileheader::size];
    toLittleEndian(Fileheader::zimMagic, header);
    toLittleEndian(getMajorVersion(), header + 4);
    toLittleEndian(getMinorVersion(), header + 6);
    std::copy(getUuid().data, getUuid().data + sizeof(Uuid), header + 8);
    toLittleEndian(getArticleCount(), header + 24);
    toLittleEndian(getClusterCount(), header + 28);
    toLittleEndian(getUrlPtrPos(), header + 32);
    toLittleEndian(getTitleIdxPos(), header + 40);
    toLittleEndian(getClusterPtrPos(), header + 48);
    toLittleEndian(getMimeListPos(), header + 56);
    toLittleEndian(getMainPage(), header + 64);
    toLittleEndian(getLayoutPage(), header + 68);
    toLittleEndian(getChecksumPos(), header + 72);

    _write(out_fd, header, Fileheader::size);
  }

  void Fileheader::read(IDataStream& ds)
  {
    uint32_t magicNumber = ds.read<uint32_t>();
    if (magicNumber != Fileheader::zimMagic)
    {
      log_error("invalid magic number " << magicNumber << " found - "
          << Fileheader::zimMagic << " expected");
      throw ZimFileFormatError("Invalid magic number");
    }

    uint16_t major_version = ds.read<uint16_t>();
    if (major_version != zimClassicMajorVersion && major_version != zimExtendedMajorVersion)
    {
      log_error("invalid zimfile major version " << major_version << " found - "
          << Fileheader::zimMajorVersion << " expected");
      throw ZimFileFormatError("Invalid version");
    }
    setMajorVersion(major_version);

    setMinorVersion(ds.read<uint16_t>());

    Uuid uuid;
    const Blob uuidBlob = ds.readBlob(sizeof(uuid.data));
    std::copy(uuidBlob.data(), uuidBlob.end(), uuid.data);
    setUuid(uuid);

    setArticleCount(ds.read<uint32_t>());
    setClusterCount(ds.read<uint32_t>());
    setUrlPtrPos(ds.read<uint64_t>());
    setTitleIdxPos(ds.read<uint64_t>());
    setClusterPtrPos(ds.read<uint64_t>());
    setMimeListPos(ds.read<uint64_t>());
    setMainPage(ds.read<uint32_t>());
    setLayoutPage(ds.read<uint32_t>());
    setChecksumPos(ds.read<uint64_t>());

    sanity_check();
  }

  void Fileheader::sanity_check() const {
    if (!!articleCount != !!clusterCount) {
      throw ZimFileFormatError("No article <=> No cluster");
    }

    if (mimeListPos != size && mimeListPos != 72) {
      throw ZimFileFormatError("mimelistPos must be 80.");
    }

    if (urlPtrPos < mimeListPos) {
      throw ZimFileFormatError("urlPtrPos must be > mimelistPos.");
    }
    if (titleIdxPos < mimeListPos) {
      throw ZimFileFormatError("titleIdxPos must be > mimelistPos.");
    }
    if (clusterPtrPos < mimeListPos) {
      throw ZimFileFormatError("clusterPtrPos must be > mimelistPos.");
    }

    if (clusterCount > articleCount) {
      throw ZimFileFormatError("Cluster count cannot be higher than article count.");
    }

    if (checksumPos != 0 && checksumPos < mimeListPos) {
      throw ZimFileFormatError("checksumPos must be > mimeListPos.");
    }
  }

}
