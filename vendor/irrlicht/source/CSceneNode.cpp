#include "ISceneNode.h"
#include "ISceneManager.h"
#include "IVideoDriver.h"
#include "irrOS.h"

namespace irr
{
     
     
    namespace scene
    {

        ISceneNode::ISceneNode(ISceneNode *parent, ISceneManager *mgr, s32 id,
                               const core::vector3df &position,
                               const core::vector3df &rotation,
                               const core::vector3df &scale)
            : RelativeTranslation(position), RelativeRotation(rotation), RelativeScale(scale),
              Parent(0), SceneManager(mgr), TriangleSelector(0), ID(id),
              AutomaticCullingState(EAC_BOX), DebugDataVisible(EDS_OFF),
              IsVisible(true), IsDebugObject(false), IsStaticObject(false)
        {
            
            Type = EST_NODE;
             if (parent)
                 parent->addChild(this);

            setAutomaticCulling(scene::EAC_OFF);
 

            updateAbsolutePosition();
        }

        ISceneNode::~ISceneNode()
        {
            removeAll();

             

            if (TriangleSelector)
                TriangleSelector->drop();
        }

        void ISceneNode::OnRegisterSceneNode()
        {
            if (IsVisible)
            {
        
                SceneManager->registerNodeForRendering(this, ESNRP_SOLID);

              

                ISceneNodeList::Iterator it = Children.begin(), end = Children.end();
                for (; it != end; ++it)
                {
                    ISceneNode *node = (*it);
                    if (node->IsVisible == true)
                        node->OnRegisterSceneNode();
                }
            }
        }

        void ISceneNode::CheckCount(int &count)
        {
            count++;
            ISceneNodeList::Iterator it = Children.begin(), end = Children.end();
            for (; it != end; ++it)
            {
                (*it)->CheckCount(count);
            }
        }

        void ISceneNode::OnAnimate(f32 deltaTime)
        {
            if (IsVisible && IsStaticObject == false)
            {
                TransformComponent* transform = getComponent<TransformComponent>();
                // If we have a transform component, update our relative transform from it
              
                updateAbsolutePosition();

                ISceneNodeList::Iterator it = Children.begin(), itEnd = Children.end();
                for (; it != itEnd; ++it)
                {
                    ISceneNode *node = (*it);
                    if (node->IsVisible == true && node->IsStaticObject == false)
                        node->OnAnimate(deltaTime);
                }


            
                    auto cmtIt = components.getIterator();
                    for (; !cmtIt.atEnd(); cmtIt++)
                    {
                        if (cmtIt.getNode()->getValue())
                            cmtIt.getNode()->getValue()->OnAnimate(deltaTime);
                    }

                  //  printf("animate components: %d\n", components.size());
            }
        }

        void ISceneNode::render()
        {

            
            if (!IsVisible)        return;
            
            //printf("render components: %d\n", components.size());
            video::IVideoDriver*    Driver = SceneManager->getVideoDriver();
            
            TransformComponent *transform = getComponent<TransformComponent>();
            MeshComponent *meshComp = getComponent<MeshComponent>();

            
            if (!transform  )  
            {
                    os::Printer::log("Missing transform component", ELL_ERROR);
                    return;
               
            }
            if (!meshComp )  
            {
                    os::Printer::log("Missing mesh component", ELL_ERROR);
                    return;
               
            }
            if (!meshComp->mesh )  
            {
                    os::Printer::log("Missing mesh in mesh component", ELL_ERROR);
                    return;
            }
            
            s32 shaderMaterialToUse = meshComp->getShaderMaterial();
            if (shaderMaterialToUse == -1)
            {
                os::Printer::log("No shader material set for mesh component", ELL_ERROR);
                return;
            }
            
            // Set world transform
            Driver->setTransform(video::ETS_WORLD, transform->getWorldMatrix());

            // Render each mesh buffer
            const u32 cnt = meshComp->mesh->getMeshBufferCount();
            for (u32 i = 0; i < cnt; ++i)
            {
                scene::IMeshBuffer *mb = meshComp->mesh->getMeshBuffer(i);

                video::SMaterial mat = mb->getMaterial();

                mat.MaterialType = (video::E_MATERIAL_TYPE)shaderMaterialToUse;

              Driver->setMaterial(mat);
              Driver->drawMeshBuffer(mb);
            }

            

        }

        const c8 *ISceneNode::getName() const { return Name.c_str(); }
        void ISceneNode::setName(const c8 *name) { Name = name; }
        void ISceneNode::setName(const core::stringc &name) { Name = name; }

        const core::aabbox3d<f32> &ISceneNode::getBoundingBox() const
        {
            return Box;
        }

        const core::aabbox3d<f32> ISceneNode::getTransformedBoundingBox() const
        {
            core::aabbox3d<f32> _box = getBoundingBox();
            AbsoluteTransformation.transformBoxEx(_box);
            return _box;
        }

        const core::matrix4 &ISceneNode::getAbsoluteTransformation() const { return AbsoluteTransformation; }

        core::matrix4 ISceneNode::getRelativeTransformation() const
        {
            core::matrix4 mat;
            mat.setRotationDegrees(RelativeRotation);
            mat.setTranslation(RelativeTranslation);

            if (RelativeScale != core::vector3df(1.f, 1.f, 1.f))
            {
                core::matrix4 smat;
                smat.setScale(RelativeScale);
                mat *= smat;
            }
            return mat;
        }

        bool ISceneNode::isVisible() const { return IsVisible; }

        bool ISceneNode::isTrulyVisible() const
        {
            if (!IsVisible)
                return false;
            if (!Parent)
                return true;
            return Parent->isTrulyVisible();
        }

        void ISceneNode::setVisible(bool isVisible) { IsVisible = isVisible; }
        void ISceneNode::setStaticNode(bool s) { IsStaticObject = s; }
        bool ISceneNode::isStaticNode() { return IsStaticObject; }

        s32 ISceneNode::getID() const { return ID; }
        void ISceneNode::setID(s32 id) { ID = id; }

        void ISceneNode::addChild(ISceneNode *child)
        {
            if (child && (child != this))
            {
                if (SceneManager != child->SceneManager)
                    child->setSceneManager(SceneManager);

                child->grab();
                child->remove();
                Children.push_back(child);
                child->Parent = this;
            }
        }

        bool ISceneNode::removeChild(ISceneNode *child)
        {
            ISceneNodeList::Iterator it = Children.begin();
            for (; it != Children.end(); ++it)
                if ((*it) == child)
                {
                    (*it)->Parent = 0;
                    (*it)->drop();
                    Children.erase(it);
                    return true;
                }
            return false;
        }

        void ISceneNode::removeAll()
        {

            for (auto it = components.getIterator(); !it.atEnd(); it++)
            {
                auto *node = it.getNode();
                if (node->getValue())
                {
                    delete node->getValue();
                    node->setValue(0);
                }
            }

            components.clear();

            ISceneNodeList::Iterator it = Children.begin();
            for (; it != Children.end(); ++it)
            {
                (*it)->Parent = 0;
                (*it)->drop();
            }
            Children.clear();
        }

        void ISceneNode::remove()
        {
            if (Parent)
                Parent->removeChild(this);
        }

      

        const core::vector3df &ISceneNode::getScale() const { return RelativeScale; }
        void ISceneNode::setScale(const core::vector3df &scale) { RelativeScale = scale; }

        const core::vector3df &ISceneNode::getRotation() const { return RelativeRotation; }
        void ISceneNode::setRotation(const core::vector3df &rotation) { RelativeRotation = rotation; }

        const core::vector3df &ISceneNode::getPosition() const { return RelativeTranslation; }
        void ISceneNode::setPosition(const core::vector3df &newpos) { RelativeTranslation = newpos; }

        core::vector3df ISceneNode::getAbsolutePosition() const { return AbsoluteTransformation.getTranslation(); }

        void ISceneNode::setAutomaticCulling(u32 state) { AutomaticCullingState = state; }
        u32 ISceneNode::getAutomaticCulling() const { return AutomaticCullingState; }

        void ISceneNode::setDebugDataVisible(u32 state) { DebugDataVisible = state; }
        u32 ISceneNode::isDebugDataVisible() const { return DebugDataVisible; }

        void ISceneNode::setIsDebugObject(bool debugObject) { IsDebugObject = debugObject; }
        bool ISceneNode::isDebugObject() const { return IsDebugObject; }

        const core::list<ISceneNode *> &ISceneNode::getChildren() const { return Children; }

        void ISceneNode::setParent(ISceneNode *newParent)
        {
            grab();
            remove();
            Parent = newParent;
            if (Parent)
                Parent->addChild(this);
            drop();
        }

        ITriangleSelector *ISceneNode::getTriangleSelector() const { return TriangleSelector; }
        void ISceneNode::setTriangleSelector(ITriangleSelector *selector)
        {
            if (TriangleSelector != selector)
            {
                if (TriangleSelector)
                    TriangleSelector->drop();
                TriangleSelector = selector;
                if (TriangleSelector)
                    TriangleSelector->grab();
            }
        }

        void ISceneNode::updateAbsolutePosition()
        {
            if (Parent)
                AbsoluteTransformation = Parent->getAbsoluteTransformation() * getRelativeTransformation();
            else
                AbsoluteTransformation = getRelativeTransformation();
        }

        scene::ISceneNode *ISceneNode::getParent() const { return Parent; }
        ESCENE_NODE_TYPE ISceneNode::getType() const { return Type; }

        void ISceneNode::serializeAttributes(io::IAttributes *out, io::SAttributeReadWriteOptions *) const
        {
            if (!out)
                return;
            out->addString("Name", Name.c_str());
            out->addInt("Id", ID);
            out->addVector3d("Position", getPosition());
            out->addVector3d("Rotation", getRotation());
            out->addVector3d("Scale", getScale());
            out->addBool("Visible", IsVisible);
            out->addInt("AutomaticCulling", AutomaticCullingState);
            out->addInt("DebugDataVisible", DebugDataVisible);
            out->addBool("IsDebugObject", IsDebugObject);
        }

        void ISceneNode::deserializeAttributes(io::IAttributes *in, io::SAttributeReadWriteOptions *)
        {
            if (!in)
                return;
            Name = in->getAttributeAsString("Name");
            ID = in->getAttributeAsInt("Id");
            setPosition(in->getAttributeAsVector3d("Position"));
            setRotation(in->getAttributeAsVector3d("Rotation"));
            setScale(in->getAttributeAsVector3d("Scale"));
            IsVisible = in->getAttributeAsBool("Visible");
            s32 tmpState = in->getAttributeAsEnumeration("AutomaticCulling", scene::AutomaticCullingNames);
            if (tmpState != -1)
                AutomaticCullingState = (u32)tmpState;
            else
                AutomaticCullingState = in->getAttributeAsInt("AutomaticCulling");
            DebugDataVisible = in->getAttributeAsInt("DebugDataVisible");
            IsDebugObject = in->getAttributeAsBool("IsDebugObject");
            updateAbsolutePosition();
        }

        ISceneNode *ISceneNode::clone(ISceneNode *, ISceneManager *) { return 0; }
        ISceneManager *ISceneNode::getSceneManager(void) const { return SceneManager; }

        void ISceneNode::cloneMembers(ISceneNode *toCopyFrom, ISceneManager *newManager)
        {
            Name = toCopyFrom->Name;
            AbsoluteTransformation = toCopyFrom->AbsoluteTransformation;
            RelativeTranslation = toCopyFrom->RelativeTranslation;
            RelativeRotation = toCopyFrom->RelativeRotation;
            RelativeScale = toCopyFrom->RelativeScale;
            ID = toCopyFrom->ID;
            setTriangleSelector(toCopyFrom->TriangleSelector);
            AutomaticCullingState = toCopyFrom->AutomaticCullingState;
            DebugDataVisible = toCopyFrom->DebugDataVisible;
            IsVisible = toCopyFrom->IsVisible;
            IsDebugObject = toCopyFrom->IsDebugObject;

            if (newManager)
                SceneManager = newManager;
            else
                SceneManager = toCopyFrom->SceneManager;

            ISceneNodeList::Iterator it = toCopyFrom->Children.begin();
            for (; it != toCopyFrom->Children.end(); ++it)
                (*it)->clone(this, newManager);

        
        }

        void ISceneNode::setSceneManager(ISceneManager *newManager)
        {
            SceneManager = newManager;
            ISceneNodeList::Iterator it = Children.begin();
            for (; it != Children.end(); ++it)
                (*it)->setSceneManager(newManager);
        }

    } // namespace scene
} // namespace irr
