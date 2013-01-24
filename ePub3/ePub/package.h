//
//  package.h
//  ePub3
//
//  Created by Jim Dovey on 2012-11-28.
//  Copyright (c) 2012-2013 The Readium Foundation.
//  
//  The Readium SDK is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef __ePub3__package__
#define __ePub3__package__

#include "epub3.h"
#include <string>
#include <vector>
#include <map>
#include <libxml/tree.h>
#include "spine.h"
#include "manifest.h"
#include "cfi.h"
#include "nav_element.h"
#include "archive_xml.h"
#include "utfstring.h"
#include "iri.h"

EPUB3_BEGIN_NAMESPACE

class Archive;
class Metadata;
class NavigationTable;

class Package
{
public:
    // indexed by property IRI
    typedef std::map<IRI, Metadata*>            MetadataMap;
    // indexed by title
    typedef std::map<string, NavigationTable*>  NavigationMap;
    // indexed by prefix
    typedef std::map<string, string>            PropertyVocabularyMap;
    
    typedef std::vector<IRI>                    PropertyList;
    
    class UnknownPrefix : public std::domain_error
    {
    public:
                    UnknownPrefix(const string &str)    : std::domain_error(str.stl_str()) {}
                    UnknownPrefix(const char* str)      : std::domain_error(str) {}
        virtual     ~UnknownPrefix() {}
    };
    
public:
                            Package() = delete;
                            Package(Archive * archive, const string& path, const string& type);
                            Package(const Package&) = delete;
                            Package(Package&&);
    virtual                 ~Package();
    
    virtual string          UniqueID()              const;
    virtual const string&   Type()                  const       { return _type; }
    virtual string          Version()               const;
    virtual const string&   BasePath()              const       { return _pathBase; }
    
    // to keep these accessible in const packages, we *must* build the tree at allocation time
    // this is open to discussion, naturally
    const MetadataMap&      Metadata()              const       { return _metadata; }
    const ManifestTable&    Manifest()              const       { return _manifest; }
    const NavigationMap&    NavigationTables()      const       { return _navigation; }
    
    const SpineItem *       FirstSpineItem()        const { return _spine.get(); }
    const SpineItem *       SpineItemAt(size_t idx) const;
    const SpineItem *       SpineItemWithIDRef(const string& idref)         const;
    size_t                  IndexOfSpineItemWithIDRef(const string& idref)  const;
    
    const ManifestItem *    ManifestItemWithID(const string& ident)         const;
    string                  CFISubpathForManifestItemWithID(const string& ident) const;
    
    const std::vector<const ManifestItem*> ManifestItemsWithProperties(PropertyList properties) const;
    
    const NavigationTable * NavigationTable(const string& title)            const;
    
    const CFI               CFIForManifestItem(const ManifestItem* item)    const;
    const CFI               CFIForSpineItem(const SpineItem* item)          const;
    
    // note that the CFI is purposely non-const so the package can correct it (cf. epub-cfi §3.5)
    const ManifestItem *    ManifestItemForCFI(CFI& cfi, CFI* pRemainingCFI) const throw (CFI::InvalidCFI);
    xmlDocPtr               DocumentForCFI(CFI& cfi, CFI* pRemainingCFI) const throw (CFI::InvalidCFI) {
        return ManifestItemForCFI(cfi, pRemainingCFI)->ReferencedDocument();
    }
    
    // array-style operators: indices get spine items, identifiers get manifest items
    const SpineItem *       operator[](size_t idx)          const   { return SpineItemAt(idx); }
    const ManifestItem *    operator[](const string& ident) const   { return ManifestItemWithID(ident); }
    
    ArchiveReader*          ReaderForRelativePath(const string& path) const {
        return _archive->ReaderAtPath((_pathBase + path).stl_str());
    }
    ArchiveXmlReader*       XmlReaderForRelativePath(const string& path) const {
        return new ArchiveXmlReader(ReaderForRelativePath(path));
    }
    
    void                    RegisterPrefixIRIStem(const string& prefix, const string& iriStem);
    IRI                     MakePropertyIRI(const string& reference, const string& prefix="")   const   throw (UnknownPrefix);
    IRI                     PropertyIRIFromAttributeValue(const string& attrValue)              const   throw (UnknownPrefix, std::invalid_argument);
    
    uint32_t                SpineCFIIndex()                 const   { return _spineCFIIndex; }
    
protected:
    Archive *               _archive;
    xmlDocPtr               _opf;
    string                  _pathBase;
    string                  _type;
    MetadataMap             _metadata;
    ManifestTable           _manifest;
    NavigationMap           _navigation;
    Auto<SpineItem>         _spine;
    
    PropertyVocabularyMap   _vocabularyLookup;
    
    // used to initialize each package's vocabulary map
    static const PropertyVocabularyMap gReservedVocabularies;
    
    // used to verify/correct CFIs
    uint32_t                _spineCFIIndex;
    
    bool                    Unpack();
    void                    InstallPrefixesFromAttributeValue(const string& attrValue);
    const SpineItem *       ConfirmOrCorrectSpineItemQualifier(const SpineItem * pItem, CFI::Component* pComponent) const;
    static NavigationList   NavTablesFromManifestItem(const ManifestItem * pItem);
};

EPUB3_END_NAMESPACE

#endif /* defined(__ePub3__package__) */
