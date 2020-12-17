/*
 * Copyright 2020 Matthieu Gautier <mgautier@kymeria.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "titleListingHandler.h"
#include "creatordata.h"

#include "../endian_tools.h"

#include <zim/writer/contentProvider.h>
#include <zim/blob.h>

using namespace zim::writer;

class ListingProvider : public ContentProvider {
  public:
    ListingProvider(const TitleListingHandler::DirentSet* dirents)
      : mp_dirents(dirents),
        m_it(dirents->begin())
    {}

    zim::size_type getSize() const override {
      return mp_dirents->size() * sizeof(zim::offset_type);
    }

    zim::Blob feed() override {
      if (m_it == mp_dirents->end()) {
        return zim::Blob(nullptr, 0);
      }
      zim::toLittleEndian((*m_it)->getIdx().v, buffer);
      m_it++;
      return zim::Blob(buffer, sizeof(zim::offset_type));
    }

  private:
    const TitleListingHandler::DirentSet* mp_dirents;
    char buffer[sizeof(zim::offset_type)];
    TitleListingHandler::DirentSet::iterator m_it;
};

TitleListingHandler::TitleListingHandler(CreatorData* data)
  : mp_creatorData(data)
{}

TitleListingHandler::~TitleListingHandler() = default;

void TitleListingHandler::start() {
}

void TitleListingHandler::stop() {
}

Dirent* TitleListingHandler::getDirent() const {
  return mp_creatorData->createDirent('W', "listing/titleOrdered/v0", "application/octet-stream++zimlisting", "");
}

std::unique_ptr<ContentProvider> TitleListingHandler::getContentProvider() const {
  return std::unique_ptr<ContentProvider>(new ListingProvider(&m_dirents));
}

void TitleListingHandler::handle(Dirent* dirent, const Hints& hints, std::shared_ptr<Item> item)
{
  m_dirents.insert(dirent);
}

