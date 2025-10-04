#include "ISceneNode.h"
#include "ISceneManager.h"
#include "IVideoDriver.h"
#include "irrOS.h"

namespace irr
{

    namespace scene
    {

        void MeshComponent::OnReady()
        {
            owner->Box.addInternalBox(mesh->getBoundingBox());
        }

        void MeshComponent::setColor(u8 r, u8 g, u8 b, u8 a)
        {
            owner->getSceneManager()->getMeshManipulator()->setVertexColors(mesh, video::SColor(255, r, g, b));
        }

        ISceneNode::ISceneNode(ISceneNode *parent, ISceneManager *mgr, s32 id,
                               const core::vector3df &position,
                               const core::vector3df &rotation,
                               const core::vector3df &scale)
            : RelativeTranslation(position), RelativeRotation(rotation), RelativeScale(scale),
              Parent(0), SceneManager(mgr), TriangleSelector(0), ID(id),
              AutomaticCullingState(EAC_BOX), DebugDataVisible(EDS_OFF),
              IsVisible(true), IsDebugObject(false), IsStaticObject(false), UseQuaternionRotation(false)
        {

            Type = EST_NODE;
            if (parent)
                parent->addChild(this);

            //           setAutomaticCulling(scene::EAC_OFF);

            RelativeOrientationQuaternion.set(RelativeRotation * core::DEGTORAD);

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

                // todo define type by compont

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

        bool ISceneNode::OnEvent(const SEvent &event)
        {
            // Pass event to components
            // printf("event components: %d\n", components.size());
            auto cmtIt = components.getIterator();
            for (; !cmtIt.atEnd(); cmtIt++)
            {
                if (cmtIt.getNode()->getValue())
                {
                    if (cmtIt.getNode()->getValue()->OnEvent(event))
                        return true;
                }
            }
            // for (auto it = Children.begin(); it != Children.end(); ++it)
            // {
            //     if ((*it)->OnEvent(event))
            //         return true;
            // }
            return false;
        }

        void ISceneNode::render()
        {

            if (!IsVisible)
                return;

            auto cmtIt = components.getIterator();
            for (; !cmtIt.atEnd(); cmtIt++)
            {
                if (cmtIt.getNode()->getValue())
                    cmtIt.getNode()->getValue()->Render();
            }

            // printf("render components: %d\n", components.size());
            video::IVideoDriver *Driver = SceneManager->getVideoDriver();
            MeshComponent *meshComp = getComponent<MeshComponent>();

            if (!meshComp)
            {
                os::Printer::log("Missing mesh component", ELL_ERROR);
                return;
            }
            if (!meshComp->mesh)
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
            Driver->setTransform(video::ETS_WORLD, getAbsoluteTransformation());

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

        void ISceneNode::setUseQuaternionRotation(bool use)
        {
            UseQuaternionRotation = use;
            if (use)
            {
                // Converter rotação atual para quaternion
                RelativeOrientationQuaternion.set(RelativeRotation * core::DEGTORAD);
            }
        }

        bool ISceneNode::isUsingQuaternionRotation() const
        {
            return UseQuaternionRotation;
        }

        void ISceneNode::setOrientation(const core::quaternion &quat)
        {
            RelativeOrientationQuaternion = quat;
            UseQuaternionRotation = true;

            // Converter para euler para compatibilidade
            core::vector3df euler;
            quat.toEuler(euler);
            RelativeRotation = euler * core::RADTODEG;
        }

        void ISceneNode::rotate(const core::quaternion &quat)
        {
            if (UseQuaternionRotation)
            {
                RelativeOrientationQuaternion = RelativeOrientationQuaternion * quat;

                core::vector3df euler;
                RelativeOrientationQuaternion.toEuler(euler);
                RelativeRotation = euler * core::RADTODEG;
            }
            else
            {

                core::quaternion current;
                current.set(RelativeRotation * core::DEGTORAD);
                current = current * quat;
                current.toEuler(RelativeRotation);
                RelativeRotation *= core::RADTODEG;
            }
        }

        core::vector3df ISceneNode::getForwardVector() const
        {
            core::matrix4 mat;
            if (UseQuaternionRotation)
                RelativeOrientationQuaternion.getMatrix(mat);
            else
                mat.setRotationDegrees(RelativeRotation);

            core::vector3df forward(0, 0, 1);
            mat.transformVect(forward);
            forward.normalize();
            return forward;
        }

        core::vector3df ISceneNode::getRightVector() const
        {
            core::matrix4 mat;
            if (UseQuaternionRotation)
                RelativeOrientationQuaternion.getMatrix(mat);
            else
                mat.setRotationDegrees(RelativeRotation);

            core::vector3df right(1, 0, 0);
            mat.transformVect(right);
            right.normalize();
            return right;
        }

        core::vector3df const ISceneNode::getUpVector() const
        {
            core::matrix4 mat;
            if (UseQuaternionRotation)
                RelativeOrientationQuaternion.getMatrix(mat);
            else
                mat.setRotationDegrees(RelativeRotation);

            core::vector3df up(0, 1, 0);
            mat.transformVect(up);
            up.normalize();
            return up;
        }

        void ISceneNode::lookAt(ISceneNode *target, const core::vector3df &up)
        {
            if (target)
                lookAt(target->getAbsolutePosition(), up);
        }

        void ISceneNode::lookAt(const core::vector3df &target, const core::vector3df &up)
        {
            core::vector3df forward = (target - getAbsolutePosition()); // Use posição absoluta
            forward.normalize();

            core::vector3df right = forward.crossProduct(up); // Ordem invertida para left-handed
            right.normalize();

            core::vector3df realUp = right.crossProduct(forward);
            realUp.normalize();

            core::matrix4 lookMatrix;
            lookMatrix[0] = right.X;
            lookMatrix[4] = right.Y;
            lookMatrix[8] = right.Z;
            lookMatrix[1] = realUp.X;
            lookMatrix[5] = realUp.Y;
            lookMatrix[9] = realUp.Z;
            lookMatrix[2] = forward.X;
            lookMatrix[6] = forward.Y;
            lookMatrix[10] = forward.Z;

            lookMatrix[3] = 0;
            lookMatrix[7] = 0;
            lookMatrix[11] = 0;
            lookMatrix[12] = 0;
            lookMatrix[13] = 0;
            lookMatrix[14] = 0;
            lookMatrix[15] = 1;

            RelativeOrientationQuaternion.set(lookMatrix);
            UseQuaternionRotation = true;

            core::vector3df euler;
            RelativeOrientationQuaternion.toEuler(euler);
            RelativeRotation = euler * core::RADTODEG;
        }

        core::quaternion ISceneNode::getOrientation() const
        {
            return RelativeOrientationQuaternion;
        }

        const core::matrix4 &ISceneNode::getAbsoluteTransformation() const { return AbsoluteTransformation; }

        core::matrix4 ISceneNode::getRelativeTransformation() const
        {

            core::matrix4 mat;

            if (UseQuaternionRotation)
            {
                RelativeOrientationQuaternion.getMatrix(mat);
            }
            else
            {
                //  euler
                mat.setRotationDegrees(RelativeRotation);
            }

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
                    node->getValue()->OnDestroy();
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

        ISceneNode *ISceneNode::clone(ISceneNode *, ISceneManager *) { return 0; }
        ISceneManager *ISceneNode::getSceneManager(void) const { return SceneManager; }

        void ISceneNode::cloneMembers(ISceneNode *toCopyFrom, ISceneManager *newManager)
        {
            Name = toCopyFrom->Name;
            AbsoluteTransformation = toCopyFrom->AbsoluteTransformation;
            RelativeTranslation = toCopyFrom->RelativeTranslation;
            RelativeRotation = toCopyFrom->RelativeRotation;
            RelativeScale = toCopyFrom->RelativeScale;
            RelativeOrientationQuaternion = toCopyFrom->RelativeOrientationQuaternion;
            ID = toCopyFrom->ID;
            UseQuaternionRotation = toCopyFrom->UseQuaternionRotation;

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

        void ISceneNode::moveForward(f32 distance)
        {
            core::vector3df forward = getForwardVector();
            RelativeTranslation += forward * distance;
        }

        void ISceneNode::moveRight(f32 distance)
        {
            core::vector3df right = getRightVector();
            RelativeTranslation += right * distance;
        }

        void ISceneNode::moveUp(f32 distance)
        {
            core::vector3df up = getUpVector();
            RelativeTranslation += up * distance;
        }

        void ISceneNode::moveLocal(const core::vector3df &localDirection)
        {
            core::vector3df forward = getForwardVector();
            core::vector3df right = getRightVector();
            core::vector3df up = getUpVector();

            core::vector3df movement =
                right * localDirection.X +
                up * localDirection.Y +
                forward * localDirection.Z;

            RelativeTranslation += movement;
        }

        void ISceneNode::yaw(f32 degrees)
        {
            core::quaternion q;
            q.fromAngleAxis(degrees * core::DEGTORAD, core::vector3df(0, 1, 0));

            if (UseQuaternionRotation)
            {
                RelativeOrientationQuaternion = RelativeOrientationQuaternion * q;
                core::vector3df euler;
                RelativeOrientationQuaternion.toEuler(euler);
                RelativeRotation = euler * core::RADTODEG;
            }
            else
            {
                RelativeRotation.Y += degrees;
            }
        }

        void ISceneNode::pitch(f32 degrees)
        {
            core::vector3df right = getRightVector();
            core::quaternion q;
            q.fromAngleAxis(degrees * core::DEGTORAD, right);

            if (UseQuaternionRotation)
            {
                RelativeOrientationQuaternion = RelativeOrientationQuaternion * q;
                core::vector3df euler;
                RelativeOrientationQuaternion.toEuler(euler);
                RelativeRotation = euler * core::RADTODEG;
            }
            else
            {
                RelativeRotation.X += degrees;
            }
        }

        void ISceneNode::roll(f32 degrees)
        {
            core::vector3df forward = getForwardVector();
            core::quaternion q;
            q.fromAngleAxis(degrees * core::DEGTORAD, forward);

            if (UseQuaternionRotation)
            {
                RelativeOrientationQuaternion = RelativeOrientationQuaternion * q;
                core::vector3df euler;
                RelativeOrientationQuaternion.toEuler(euler);
                RelativeRotation = euler * core::RADTODEG;
            }
            else
            {
                RelativeRotation.Z += degrees;
            }
        }

        void ISceneNode::rotatePitchYawRoll(f32 pitch, f32 yaw, f32 roll)
        {
            // Ordem comum: Yaw -> Pitch -> Roll
            core::quaternion qYaw, qPitch, qRoll;

            qYaw.fromAngleAxis(yaw * core::DEGTORAD, core::vector3df(0, 1, 0));
            qPitch.fromAngleAxis(pitch * core::DEGTORAD, getRightVector());
            qRoll.fromAngleAxis(roll * core::DEGTORAD, getForwardVector());

            core::quaternion combined = qYaw * qPitch * qRoll;

            if (UseQuaternionRotation)
            {
                RelativeOrientationQuaternion = RelativeOrientationQuaternion * combined;
                core::vector3df euler;
                RelativeOrientationQuaternion.toEuler(euler);
                RelativeRotation = euler * core::RADTODEG;
            }
            else
            {
                RelativeRotation.X += pitch;
                RelativeRotation.Y += yaw;
                RelativeRotation.Z += roll;
            }
        }

        void ISceneNode::rotateAroundLocalAxis(const core::vector3df &axis, f32 degrees)
        {
            core::vector3df worldAxis = transformVectorByOrientation(axis);
            core::quaternion q;
            q.fromAngleAxis(degrees * core::DEGTORAD, worldAxis);
            rotate(q);
        }

        void ISceneNode::moveTowards(const core::vector3df &target, f32 distance)
        {
            core::vector3df direction = target - getAbsolutePosition();
            f32 currentDistance = direction.getLength();

            if (currentDistance > 0)
            {
                direction.normalize();
                f32 moveDistance = core::min_(distance, currentDistance);
                RelativeTranslation += direction * moveDistance;
            }
        }

        void ISceneNode::rotateTowards(const core::vector3df &target, f32 maxDegrees)
        {
            core::vector3df currentForward = getForwardVector();
            core::vector3df targetDirection = target - getAbsolutePosition();
            targetDirection.normalize();

            // Calcula o ângulo entre as direções
            f32 dot = currentForward.dotProduct(targetDirection);
            f32 angle = acos(core::clamp(dot, -1.0f, 1.0f)) * core::RADTODEG;

            if (angle > 0.1f) // Evita jitter
            {
                // Eixo de rotação
                core::vector3df axis = currentForward.crossProduct(targetDirection);
                axis.normalize();

                // Limita a rotação
                f32 rotationAngle = core::min_(maxDegrees, angle);

                core::quaternion q;
                q.fromAngleAxis(rotationAngle * core::DEGTORAD, axis);
                rotate(q);
            }
        }

        void ISceneNode::slerpOrientation(const core::quaternion &target, f32 factor)
        {
            UseQuaternionRotation = true;
            RelativeOrientationQuaternion.slerp(RelativeOrientationQuaternion, target, factor);

            core::vector3df euler;
            RelativeOrientationQuaternion.toEuler(euler);
            RelativeRotation = euler * core::RADTODEG;
        }

        static inline void rotateByQuat(core::vector3df &v, const core::quaternion &q)
        {
            core::matrix4 m;
            q.getMatrix(m);  // só rotação
            m.rotateVect(v); // v = q * v * q^{-1}
        }

        void ISceneNode::orbitAround(const core::vector3df &center, f32 yawDegrees, f32 pitchDegrees)
        {
            core::vector3df offset = RelativeTranslation - center;
            if (offset.getLengthSQ() < 1e-12f)
                offset.set(0, 0, 1); // evita raio zero

            core::quaternion qYaw;
            qYaw.fromAngleAxis(yawDegrees * core::DEGTORAD, core::vector3df(0, 1, 0));
            rotateByQuat(offset, qYaw);

            core::vector3df up(0, 1, 0);
            core::vector3df right = up.crossProduct(offset);
            if (right.getLengthSQ() > 1e-12f)
                right.normalize();
            else
                right.set(1, 0, 0);

            core::quaternion qPitch;
            qPitch.fromAngleAxis(pitchDegrees * core::DEGTORAD, right);
            rotateByQuat(offset, qPitch);

            RelativeTranslation = center + offset;
            lookAt(center);
        }

        core::vector3df ISceneNode::transformVectorByOrientation(const core::vector3df &vec) const
        {
            core::matrix4 mat;
            if (UseQuaternionRotation)
            {
                RelativeOrientationQuaternion.getMatrix(mat);
            }
            else
            {
                mat.setRotationDegrees(RelativeRotation);
            }

            core::vector3df result = vec;
            mat.transformVect(result);
            return result;
        }

        f32 ISceneNode::getDistanceTo(ISceneNode *other) const
        {
            if (!other)
                return 0.0f;
            return getAbsolutePosition().getDistanceFrom(other->getAbsolutePosition());
        }

        f32 ISceneNode::getAngleTo(ISceneNode *other) const
        {
            if (!other)
                return 0.0f;

            core::vector3df dir = other->getAbsolutePosition() - getAbsolutePosition();
            return atan2(dir.X, dir.Z) * core::RADTODEG;
        }

        f32 ISceneNode::getYaw() const
        {
            if (UseQuaternionRotation)
            {
                core::vector3df euler;
                RelativeOrientationQuaternion.toEuler(euler);
                return euler.Y * core::RADTODEG;
            }
            return RelativeRotation.Y;
        }

        f32 ISceneNode::getPitch() const
        {
            if (UseQuaternionRotation)
            {
                core::vector3df euler;
                RelativeOrientationQuaternion.toEuler(euler);
                return euler.X * core::RADTODEG;
            }
            return RelativeRotation.X;
        }

        f32 ISceneNode::getRoll() const
        {
            if (UseQuaternionRotation)
            {
                core::vector3df euler;
                RelativeOrientationQuaternion.toEuler(euler);
                return euler.Z * core::RADTODEG;
            }
            return RelativeRotation.Z;
        }

        core::vector3df ISceneNode::getEulerAngles() const
        {
            if (UseQuaternionRotation)
            {
                core::vector3df euler;
                RelativeOrientationQuaternion.toEuler(euler);
                return euler * core::RADTODEG;
            }
            return RelativeRotation;
        }

        core::vector3df ISceneNode::getEulerAnglesRadians() const
        {
            if (UseQuaternionRotation)
            {
                core::vector3df euler;
                RelativeOrientationQuaternion.toEuler(euler);
                return euler;
            }
            return RelativeRotation * core::DEGTORAD;
        }

        bool ISceneNode::isLookingAt(const core::vector3df &target, f32 tolerance) const
        {
            core::vector3df forward = getForwardVector();
            core::vector3df toTarget = target - getAbsolutePosition();
            toTarget.normalize();

            f32 dot = forward.dotProduct(toTarget);
            f32 angle = acos(core::clamp(dot, -1.0f, 1.0f)) * core::RADTODEG;

            return angle <= tolerance;
        }

        bool ISceneNode::isInFieldOfView(ISceneNode *other, f32 fovDegrees) const
        {
            if (!other)
                return false;
            return isLookingAt(other->getAbsolutePosition(), fovDegrees * 0.5f);
        }

        core::vector3df ISceneNode::getDirectionTo(const core::vector3df &target) const
        {
            core::vector3df dir = target - getAbsolutePosition();
            dir.normalize();
            return dir;
        }

        core::vector3df ISceneNode::getDirectionTo(ISceneNode *other) const
        {
            if (!other)
                return core::vector3df(0, 0, 0);
            return getDirectionTo(other->getAbsolutePosition());
        }

        core::vector3df ISceneNode::getVelocityToReach(const core::vector3df &target, f32 time) const
        {
            if (time <= 0.0f)
                return core::vector3df(0, 0, 0);
            return (target - getAbsolutePosition()) / time;
        }

        // ============================================
        // UTILIDADES DE INTERPOLAÇÃO
        // ============================================

        void ISceneNode::lerpPosition(const core::vector3df &target, f32 factor)
        {
            factor = core::clamp(factor, 0.0f, 1.0f);
            RelativeTranslation = RelativeTranslation.getInterpolated(target, factor);
        }

        void ISceneNode::lerpRotation(const core::vector3df &targetRotation, f32 factor)
        {
            factor = core::clamp(factor, 0.0f, 1.0f);

            if (UseQuaternionRotation)
            {
                core::quaternion targetQuat;
                targetQuat.set(targetRotation * core::DEGTORAD);
                slerpOrientation(targetQuat, factor);
            }
            else
            {
                RelativeRotation = RelativeRotation.getInterpolated(targetRotation, factor);
            }
        }

        void ISceneNode::lerpScale(const core::vector3df &targetScale, f32 factor)
        {
            factor = core::clamp(factor, 0.0f, 1.0f);
            RelativeScale = RelativeScale.getInterpolated(targetScale, factor);
        }

        void ISceneNode::lerpTransform(ISceneNode *target, f32 factor)
        {
            if (!target)
                return;

            lerpPosition(target->getPosition(), factor);
            lerpRotation(target->getRotation(), factor);
            lerpScale(target->getScale(), factor);
        }

        void ISceneNode::alignToNormal(const core::vector3df &normal, const core::vector3df &up)
        {
            core::vector3df normNormal = normal;
            normNormal.normalize();

            core::vector3df right = up.crossProduct(normNormal);
            right.normalize();

            core::vector3df realUp = normNormal.crossProduct(right);

            core::matrix4 mat;
            mat[0] = right.X;
            mat[4] = right.Y;
            mat[8] = right.Z;
            mat[1] = realUp.X;
            mat[5] = realUp.Y;
            mat[9] = realUp.Z;
            mat[2] = normNormal.X;
            mat[6] = normNormal.Y;
            mat[10] = normNormal.Z;
            mat[3] = 0;
            mat[7] = 0;
            mat[11] = 0;
            mat[12] = 0;
            mat[13] = 0;
            mat[14] = 0;
            mat[15] = 1;

            RelativeOrientationQuaternion.set(mat);
            UseQuaternionRotation = true;

            core::vector3df euler;
            RelativeOrientationQuaternion.toEuler(euler);
            RelativeRotation = euler * core::RADTODEG;
        }

        void ISceneNode::alignUpVector(const core::vector3df &worldUp)
        {
            core::vector3df forward = getForwardVector();
            forward.Y = 0; // Projeta no plano horizontal
            forward.normalize();

            core::vector3df right = worldUp.crossProduct(forward);
            right.normalize();

            core::vector3df realForward = right.crossProduct(worldUp);

            core::matrix4 mat;
            mat[0] = right.X;
            mat[4] = right.Y;
            mat[8] = right.Z;
            mat[1] = worldUp.X;
            mat[5] = worldUp.Y;
            mat[9] = worldUp.Z;
            mat[2] = realForward.X;
            mat[6] = realForward.Y;
            mat[10] = realForward.Z;
            mat[3] = 0;
            mat[7] = 0;
            mat[11] = 0;
            mat[12] = 0;
            mat[13] = 0;
            mat[14] = 0;
            mat[15] = 1;

            RelativeOrientationQuaternion.set(mat);
            UseQuaternionRotation = true;

            core::vector3df euler;
            RelativeOrientationQuaternion.toEuler(euler);
            RelativeRotation = euler * core::RADTODEG;
        }

        void ISceneNode::resetRotation()
        {
            RelativeRotation = core::vector3df(0, 0, 0);
            if (UseQuaternionRotation)
            {
                RelativeOrientationQuaternion.makeIdentity();
            }
        }

        void ISceneNode::resetOrientation()
        {
            RelativeOrientationQuaternion.makeIdentity();
            RelativeRotation = core::vector3df(0, 0, 0);
            UseQuaternionRotation = true;
        }

        bool ISceneNode::isNearPosition(const core::vector3df &target, f32 tolerance) const
        {
            return getAbsolutePosition().getDistanceFrom(target) <= tolerance;
        }

        bool ISceneNode::isNearRotation(const core::vector3df &targetRotation, f32 tolerance) const
        {
            core::vector3df current = getEulerAngles();
            core::vector3df diff = targetRotation - current;

            // Normaliza ângulos para -180 a 180
            while (diff.X > 180.0f)
                diff.X -= 360.0f;
            while (diff.X < -180.0f)
                diff.X += 360.0f;
            while (diff.Y > 180.0f)
                diff.Y -= 360.0f;
            while (diff.Y < -180.0f)
                diff.Y += 360.0f;
            while (diff.Z > 180.0f)
                diff.Z -= 360.0f;
            while (diff.Z < -180.0f)
                diff.Z += 360.0f;

            return (fabs(diff.X) <= tolerance &&
                    fabs(diff.Y) <= tolerance &&
                    fabs(diff.Z) <= tolerance);
        }

        core::vector3df ISceneNode::getAngularVelocityToReach(const core::vector3df &targetRotation, f32 time) const
        {
            if (time <= 0.0f)
                return core::vector3df(0, 0, 0);

            core::vector3df current = getEulerAngles();
            core::vector3df diff = targetRotation - current;

            // Normaliza para o caminho mais curto
            while (diff.X > 180.0f)
                diff.X -= 360.0f;
            while (diff.X < -180.0f)
                diff.X += 360.0f;
            while (diff.Y > 180.0f)
                diff.Y -= 360.0f;
            while (diff.Y < -180.0f)
                diff.Y += 360.0f;
            while (diff.Z > 180.0f)
                diff.Z -= 360.0f;
            while (diff.Z < -180.0f)
                diff.Z += 360.0f;

            return diff / time;
        }

        // ============================================
        // UTILIDADES DE ESPAÇO LOCAL/GLOBAL
        // ============================================

        core::vector3df ISceneNode::localToWorld(const core::vector3df &localPos) const
        {
            core::matrix4 mat = getAbsoluteTransformation();
            core::vector3df result = localPos;
            mat.transformVect(result);
            return result;
        }

        core::vector3df ISceneNode::worldToLocal(const core::vector3df &worldPos) const
        {
            core::matrix4 mat = getAbsoluteTransformation();
            mat.makeInverse();
            core::vector3df result = worldPos;
            mat.transformVect(result);
            return result;
        }

        core::vector3df ISceneNode::localToWorldDirection(const core::vector3df &localDir) const
        {
            return transformVectorByOrientation(localDir);
        }

        core::vector3df ISceneNode::worldToLocalDirection(const core::vector3df &worldDir) const
        {
            core::matrix4 mat;
            if (UseQuaternionRotation)
            {
                RelativeOrientationQuaternion.getMatrix(mat);
            }
            else
            {
                mat.setRotationDegrees(RelativeRotation);
            }
            mat.makeInverse();

            core::vector3df result = worldDir;
            mat.rotateVect(result);
            return result;
        }

        core::vector3df ISceneNode::transformPoint(const core::vector3df &localPoint) const
        {
            core::matrix4 mat = getAbsoluteTransformation();
            core::vector3df result;
            mat.transformVect(result, localPoint);
            return result;
        }

        // Converte um ponto de espaço world para espaço local
        core::vector3df ISceneNode::inverseTransformPoint(const core::vector3df &worldPoint) const
        {
            core::matrix4 mat = getAbsoluteTransformation();
            core::matrix4 invMat;
            mat.getInverse(invMat);
            core::vector3df result;
            invMat.transformVect(result, worldPoint);
            return result;
        }

        // Converte uma direção de espaço local para world (ignora posição/escala)
        core::vector3df ISceneNode::transformDirection(const core::vector3df &localDir) const
        {
            core::matrix4 mat = getAbsoluteTransformation();
            core::vector3df result;
            mat.rotateVect(result, localDir);
            return result.normalize();
        }

        // Converte uma direção de espaço world para local (ignora posição/escala)
        core::vector3df ISceneNode::inverseTransformDirection(const core::vector3df &worldDir) const
        {
            core::matrix4 mat = getAbsoluteTransformation();
            core::matrix4 invMat;
            mat.getInverse(invMat);
            core::vector3df result;
            invMat.rotateVect(result, worldDir);
            return result.normalize();
        }

        // Converte um vetor de espaço local para world (mantém escala, ignora posição)
        core::vector3df ISceneNode::transformVector(const core::vector3df &localVec) const
        {
            core::matrix4 mat = getAbsoluteTransformation();
            core::vector3df result;
            mat.rotateVect(result, localVec);
            return result;
        }

        // Converte um vetor de espaço world para local (mantém escala, ignora posição)
        core::vector3df ISceneNode::inverseTransformVector(const core::vector3df &worldVec) const
        {
            core::matrix4 mat = getAbsoluteTransformation();
            core::matrix4 invMat;
            mat.getInverse(invMat);
            core::vector3df result;
            invMat.rotateVect(result, worldVec);
            return result;
        }

        core::vector3df ISceneNode::smoothDamp(
            const core::vector3df &current,
            const core::vector3df &target,
            core::vector3df &currentVelocity,
            f32 smoothTime,
            f32 maxSpeed,
            f32 deltaTime) const
        {
            smoothTime = core::max_(0.0001f, smoothTime);
            f32 omega = 2.0f / smoothTime;
            f32 x = omega * deltaTime;
            f32 exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

            core::vector3df change = current - target;
            core::vector3df originalTo = target;

            // Limita velocidade máxima
            f32 maxChange = maxSpeed * smoothTime;
            f32 maxChangeSq = maxChange * maxChange;
            f32 sqrMag = change.getLengthSQ();
            if (sqrMag > maxChangeSq)
            {
                f32 mag = core::squareroot(sqrMag);
                change = change * (maxChange / mag);
            }

            core::vector3df newTarget = current - change;
            core::vector3df temp = (currentVelocity + omega * change) * deltaTime;
            currentVelocity = (currentVelocity - omega * temp) * exp;
            core::vector3df output = newTarget + (change + temp) * exp;

            // Previne overshoot
            core::vector3df origMinusCurrent = originalTo - current;
            core::vector3df outMinusOrig = output - originalTo;
            if (origMinusCurrent.dotProduct(outMinusOrig) > 0)
            {
                output = originalTo;
                currentVelocity = (output - originalTo) / deltaTime;
            }

            return output;
        }

        void ISceneNode::smoothDampPosition(
            const core::vector3df &target,
            f32 smoothTime,
            f32 maxSpeed,
            f32 deltaTime)
        {
            static core::map<ISceneNode *, core::vector3df> velocities;
            auto node = velocities.find(this);
            core::vector3df velocity(0, 0, 0);
            if (node)
                velocity = node->getValue();

            core::vector3df current = getPosition();
            core::vector3df newPos = smoothDamp(current, target, velocity, smoothTime, maxSpeed, deltaTime);
            setPosition(newPos);

            if (node)
                node->setValue(velocity);
            else
                velocities.insert(this, velocity);
        }

        core::vector3df ISceneNode::moveTowardsSmooth(
            const core::vector3df &current,
            const core::vector3df &target,
            f32 &currentSpeed,
            f32 maxSpeed,
            f32 acceleration,
            f32 deceleration,
            f32 deltaTime) const
        {
            core::vector3df direction = target - current;
            f32 distance = direction.getLength();

            if (distance < 0.0001f)
            {
                currentSpeed = 0;
                return target;
            }

            direction.normalize();

            // Calcula distância de travagem
            f32 brakingDistance = (currentSpeed * currentSpeed) / (2.0f * deceleration);

            // Acelera ou desacelera
            if (distance > brakingDistance)
            {
                // Acelera
                currentSpeed += acceleration * deltaTime;
                currentSpeed = core::min_(currentSpeed, maxSpeed);
            }
            else
            {
                // Desacelera
                currentSpeed -= deceleration * deltaTime;
                currentSpeed = core::max_(currentSpeed, 0.0f);
            }

            // Move
            f32 moveDistance = currentSpeed * deltaTime;
            if (moveDistance >= distance)
            {
                currentSpeed = 0;
                return target;
            }

            return current + direction * moveDistance;
        }

        void ISceneNode::moveTowardsWithAcceleration(
            const core::vector3df &target,
            f32 maxSpeed,
            f32 acceleration,
            f32 deceleration,
            f32 deltaTime)
        {
            static core::map<ISceneNode *, f32> speeds;
            auto node = speeds.find(this);
            f32 currentSpeed = node ? node->getValue() : 0.0f;

            core::vector3df current = getAbsolutePosition();
            core::vector3df newPos = moveTowardsSmooth(
                current, target, currentSpeed,
                maxSpeed, acceleration, deceleration, deltaTime);

            setPosition(newPos);

            if (node)
                node->setValue(currentSpeed);
            else
                speeds.insert(this, currentSpeed);
        }

        core::quaternion ISceneNode::rotateTowardsQuat(
            const core::quaternion &from,
            const core::quaternion &to,
            f32 maxDegreesPerSecond,
            f32 deltaTime) const
        {
            f32 angle = from.dotProduct(to);

            // Já está no target
            if (angle >= 0.9999f)
                return to;

            // Calcula o ângulo entre quaternions
            f32 angleInDegrees = core::radToDeg(2.0f * acos(core::clamp(angle, -1.0f, 1.0f)));

            // Limita rotação
            f32 maxRotation = maxDegreesPerSecond * deltaTime;
            if (angleInDegrees <= maxRotation)
                return to;

            // Interpola
            f32 t = maxRotation / angleInDegrees;
            core::quaternion result;
            result.slerp(from, to, t);
            return result;
        }

        void ISceneNode::rotateTowardsDirection(
            const core::vector3df &targetDirection,
            f32 maxDegreesPerSecond,
            f32 deltaTime,
            const core::vector3df &up)
        {
            if (targetDirection.getLengthSQ() < 0.0001f)
                return;

            core::vector3df dir = targetDirection;
            dir.normalize();

            // Cria quaternion do target
            core::matrix4 targetMat;
            targetMat.buildCameraLookAtMatrixLH(
                core::vector3df(0, 0, 0),
                dir,
                up);
            core::quaternion targetQuat(targetMat);

            // Rotaciona suavemente
            core::quaternion currentQuat = getOrientation();
            core::quaternion newQuat = rotateTowardsQuat(
                currentQuat, targetQuat,
                maxDegreesPerSecond, deltaTime);

            setOrientation(newQuat);
        }

        void ISceneNode::rotateTowardsNode(
            ISceneNode *target,
            f32 maxDegreesPerSecond,
            f32 deltaTime,
            const core::vector3df &up)
        {
            if (!target)
                return;

            core::vector3df direction = target->getAbsolutePosition() - getAbsolutePosition();
            rotateTowardsDirection(direction, maxDegreesPerSecond, deltaTime, up);
        }

        // Normaliza diferença de ângulo para [-180, 180]
        static inline f32 wrapDeltaDeg(f32 d)
        {
            while (d > 180.0f)
                d -= 360.0f;
            while (d < -180.0f)
                d += 360.0f;
            return d;
        }

        void ISceneNode::rotateTowardsEuler(const core::vector3df &targetRotation,
                                            f32 maxDegreesPerSecond,
                                            f32 deltaTime)
        {
            const core::vector3df current = getRotation();

            // diffs normalizados para [-180, 180]
            const f32 dx = wrapDeltaDeg(targetRotation.X - current.X);
            const f32 dy = wrapDeltaDeg(targetRotation.Y - current.Y);
            const f32 dz = wrapDeltaDeg(targetRotation.Z - current.Z);

            const f32 maxStep = maxDegreesPerSecond * deltaTime;

            core::vector3df next = current;

            next.X = (core::abs_(dx) <= maxStep) ? targetRotation.X
                                                 : (current.X + core::clamp(dx, -maxStep, maxStep));
            next.Y = (core::abs_(dy) <= maxStep) ? targetRotation.Y
                                                 : (current.Y + core::clamp(dy, -maxStep, maxStep));
            next.Z = (core::abs_(dz) <= maxStep) ? targetRotation.Z
                                                 : (current.Z + core::clamp(dz, -maxStep, maxStep));

            setRotation(next);

            // Se também manténs orientação por quaternion, sincroniza aqui:
            if (isUsingQuaternionRotation())
            {
                core::quaternion q;
                q.fromEuler(next * core::DEGTORAD); // Irrlicht usa ordem (X,Y,Z) em rad
                setOrientation(q);                  // ou atualiza o teu RelativeOrientationQuaternion direto
            }
        }

        void ISceneNode::smoothLookAt(
            const core::vector3df &target,
            f32 rotationSpeed,
            f32 deltaTime,
            const core::vector3df &up)
        {
            core::vector3df direction = target - getAbsolutePosition();
            rotateTowardsDirection(direction, rotationSpeed, deltaTime, up);
        }

        void ISceneNode::smoothLookAtNode(
            ISceneNode *target,
            f32 rotationSpeed,
            f32 deltaTime,
            const core::vector3df &up)
        {
            if (!target)
                return;
            smoothLookAt(target->getAbsolutePosition(), rotationSpeed, deltaTime, up);
        }

    } // namespace scene
} // namespace irr
