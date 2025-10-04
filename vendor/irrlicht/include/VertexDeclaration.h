 
#ifndef __VERTEX_DECLARATION_H__
#define __VERTEX_DECLARATION_H__

#include "VertexElement.h"
#include "IReferenceCounted.h"
#include "irrArray.h"

namespace irr
{
    namespace video
    {
        class VertexDeclaration : public IReferenceCounted
        {
        public:
            core::array<VertexElement> Elements;

            //! Add a vertex element
            const VertexElement &addElement(u16 source, u32 offset,
                                            E_VERTEX_ELEMENT_TYPE type,
                                            E_VERTEX_ELEMENT_SEMANTIC semantic,
                                            u16 index = 0)
            {
                VertexElement elem;
                elem.Source = source;
                elem.Offset = offset;
                elem.Type = type;
                elem.Semantic = semantic;
                elem.Index = index;
                Elements.push_back(elem);
                return Elements.getLast();
            }

            //! Insert element at specific position
            const VertexElement &insertElement(u32 atPosition, u16 source, u32 offset,
                                               E_VERTEX_ELEMENT_TYPE type,
                                               E_VERTEX_ELEMENT_SEMANTIC semantic,
                                               u16 index = 0)
            {
                VertexElement elem;
                elem.Source = source;
                elem.Offset = offset;
                elem.Type = type;
                elem.Semantic = semantic;
                elem.Index = index;

                if (atPosition >= Elements.size())
                    Elements.push_back(elem);
                else
                    Elements.insert(elem, atPosition);

                return Elements[atPosition];
            }

            //! Remove element
            void removeElement(E_VERTEX_ELEMENT_SEMANTIC semantic, u16 index = 0)
            {
                for (u32 i = 0; i < Elements.size(); ++i)
                {
                    if (Elements[i].Semantic == semantic && Elements[i].Index == index)
                    {
                        Elements.erase(i);
                        return;
                    }
                }
            }

            //! Remove element by index
            void removeElement(u32 elemIndex)
            {
                if (elemIndex < Elements.size())
                    Elements.erase(elemIndex);
            }

            //! Find element by semantic
            const VertexElement *findElementBySemantic(
                E_VERTEX_ELEMENT_SEMANTIC semantic, u16 index = 0) const
            {
                for (u32 i = 0; i < Elements.size(); ++i)
                {
                    if (Elements[i].Semantic == semantic && Elements[i].Index == index)
                        return &Elements[i];
                }
                return nullptr;
            }

            //! Get vertex size for a specific buffer source
            u32 getVertexSize(u16 source) const
            {
                u32 size = 0;
                for (u32 i = 0; i < Elements.size(); ++i)
                {
                    if (Elements[i].Source == source)
                        size += Elements[i].getSize();
                }
                return size;
            }

            //! Get all elements for a specific source
            core::array<const VertexElement *> findElementsBySource(u16 source) const
            {
                core::array<const VertexElement *> result;
                for (u32 i = 0; i < Elements.size(); ++i)
                {
                    if (Elements[i].Source == source)
                        result.push_back(&Elements[i]);
                }
                return result;
            }

            //! Clone declaration
            VertexDeclaration *clone() const
            {
                VertexDeclaration *newDecl = new VertexDeclaration();
                newDecl->Elements = Elements;
                return newDecl;
            }

            //! Get element count
            u32 getElementCount() const { return Elements.size(); }

            //! Get element by index
            const VertexElement &getElement(u32 index) const { return Elements[index]; }
        };

    } // namespace scene
} // namespace irr

#endif