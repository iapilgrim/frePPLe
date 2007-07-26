/***************************************************************************
  file : $HeadURL$
  version : $LastChangedRevision$  $LastChangedBy$
  date : $LastChangedDate$
  email : jdetaeye@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 * Copyright (C) 2007 by Johan De Taeye                                    *
 *                                                                         *
 * This library is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2.1 of the License, or  *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser *
 * General Public License for more details.                                *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA *
 *                                                                         *
 ***************************************************************************/


template <class T> inline ostream& operator << (ostream &o, const Object::RLock<T> &n)
{
  return o << &*n;
}


template <class T> inline ostream& operator << (ostream &o, const Object::WLock<T> &n)
{
  return o << &*n;
}


template <class T> inline ostream& operator << (ostream &o, const HasName<T> &n)
{
  return o << n.getName();
}


template <class T> inline ostream& operator << (ostream &o, const HasName<T> *n)
{
  return o << (n ? n->getName() : string("NULL"));
}


template <class T> void HasHierarchy<T>::setOwner (T* fam)
{
  // Check if already set to the same entity
  if (parent == fam) return;

  // Avoid loops in the hierarchy. For instance, HasHierarchy A points to B
  // as its owner, and B points to A.
  for (T *t = fam; t; t = t->parent)
    if (t == this)
      throw LogicException("Invalid hierarchy relation between \""
          + this->getName() + "\" and \"" + fam->getName() + "\"");

  // Clean up previous owner, if any
  if (parent)
  {
    if (parent->first_child == this)
      // We are the first child of our parent
      parent->first_child = next_brother;
    else
    {
      // Removed somewhere in the middle of the list of children
      T *i = parent->first_child;
      while (i && i->next_brother!=this) i = i->next_brother;
      if (!i) throw LogicException("Invalid hierarchy data");
      i->next_brother = next_brother;
    }
  }

  // Set new owner
  parent = fam;

  // Register the new member at the owner
  if (fam)
  {
    if (fam->first_child)
    {
      // We append it at the end of the list, preserving the insert order.
      T *i = fam->first_child;
      while (i->next_brother) i = i->next_brother;
      i->next_brother = static_cast<T*>(this);
    }
    else
      // I am the first child of my parent
      fam->first_child = static_cast<T*>(this);
  }
}


template <class T> void HasHierarchy<T>::writeElement
(XMLOutput* o, const XMLtag &t, mode m) const
{
  /** Note that this function is never called on its own. It is always called
    * from the writeElement() method of a subclass. Therefore we don't need
    * to worry about the refOnly or incHeader parameters.
    */
  assert(m == DEFAULT);
  // Note that nothing is written when the owner is NULL
  o->writeElement(Tags::tag_owner, parent);
  if (first_child)
  {
    o->BeginObject (Tags::tag_members);
    for (T *i = first_child; i; i=i->next_brother)
      o->writeElement(*(T::metadata.typetag), i);
    o->EndObject (Tags::tag_members);
  }
}


template <class T> void HasHierarchy<T>::beginElement
(XMLInput& pIn, XMLElement& pElement)
{
  if (pElement.isA(Tags::tag_owner) ||
      (pIn.getParentElement().isA(Tags::tag_members)
       && pElement.isA(T::metadata.typetag)))
    // Start reading a member of the parent
    pIn.readto( T::reader(T::metadata,pIn) );
}


template <class T> void HasHierarchy<T>::endElement (XMLInput& pIn,
    XMLElement& pElement)
{
  if (pElement.isA(Tags::tag_owner) && !pIn.isObjectEnd())
  {
    // we just ended an owner element: ...<OWNER>abc</OWNER>
    T* o = dynamic_cast<T*>(pIn.getPreviousObject());
    if (o) setOwner(o);
  }
  else if (pElement.isA(T::metadata.typetag)
     && pIn.getParentElement().isA(Tags::tag_members)
     && pIn.isObjectEnd() )
  {
    // we just have ended a member element <MEMBERS><TAG>abc<TAG>...</MEMBERS>
    T* o = dynamic_cast<T*>(pIn.getParentObject());
    if (o) setOwner(o);
  }
}


template <class T> HasHierarchy<T>::~HasHierarchy()
{
  // All my members now point to my parent.
  T* last_child = NULL;
  for (T *i = first_child; i; i=i->next_brother)
  {
    i->parent = parent;
    last_child = i;
  }

  if (parent && last_child)
  {
    // Extend the child list of my parent.
    // The new children are prepended to the list of existing children.
    last_child->next_brother = parent->first_child;
    parent->first_child = first_child;
  }
  if (!parent)
  {
    // If there is no new parent, we also clear the next-brother field of
    // the children.
    T* j;
    for (T *i = first_child; i; i=j)
    {
      j = i->next_brother;
      i->next_brother = NULL;
    }
  }
  else
    // A parent exists and I have to remove my as a member
    setOwner(NULL);
}


template <class T> unsigned short HasHierarchy<T>::getHierarchyLevel() const
{
  unsigned short i(0);
  for (const T* p = this; p->parent; p = p->parent) ++i;
  return i;
}