// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_SCENE_NODE_H_INCLUDED__
#define __I_SCENE_NODE_H_INCLUDED__
#include <typeinfo>

#include "ESceneNodeTypes.h"
#include "ECullingTypes.h"
#include "IEventReceiver.h"
#include "EDebugSceneTypes.h"
#include "ITriangleSelector.h"
#include "SMaterial.h"
#include "irrString.h"
#include "IMesh.h"
#include "aabbox3d.h"
#include "matrix4.h"
#include "irrList.h"
#include "irrArray.h"
#include "irrMap.h"
#include "IAttributes.h"

namespace irr
{
	namespace scene
	{
		class ISceneManager;
		class ISceneNode;

		//! Typedef for list of scene nodes
		typedef core::list<ISceneNode *> ISceneNodeList;

		class IComponent
		{

		protected:
			friend class ISceneNode;
			ISceneNode *owner = nullptr;

		public:
			virtual ~IComponent() = default;
			virtual void OnAnimate(f32 deltaTime) {}
			virtual void Render() {}
			virtual void OnReady() {}
			virtual void OnDestroy() {}
			virtual bool OnEvent(const SEvent &event) { return false; };
		};

		class MeshComponent : public IComponent
		{
			scene::IMesh *mesh = nullptr;
			s32 shaderMaterial = -1;
			friend class ISceneNode;

		public:
			MeshComponent(scene::IMesh *m)
				: mesh(m)
			{
				if (mesh)
					mesh->grab();
			}

			void OnReady() override;

			void setShaderMaterial(s32 materialType)
			{
				shaderMaterial = materialType;
			}

			void setColor(u8 r, u8 g, u8 b, u8 a = 255);

			s32 getShaderMaterial() const { return shaderMaterial; }
			
			scene::IMesh *getMesh() const {return mesh;}


			~MeshComponent()
			{
				if (mesh)
					mesh->drop();
			}
		};

		//! Scene node interface.
		class ISceneNode : public virtual IReferenceCounted
		{
		public:
			//! Constructor
			ISceneNode(ISceneNode *parent, ISceneManager *mgr, s32 id = -1,
					   const core::vector3df &position = core::vector3df(0, 0, 0),
					   const core::vector3df &rotation = core::vector3df(0, 0, 0),
					   const core::vector3df &scale = core::vector3df(1.0f, 1.0f, 1.0f));

			//! Destructor
			virtual ~ISceneNode();

			//! This method is called just before the rendering process of the whole scene.
			virtual void OnRegisterSceneNode();

			virtual void CheckCount(int &count);

			//! OnAnimate() is called just before rendering the whole scene.
			virtual void OnAnimate(f32 deltaTime);

			virtual bool OnEvent(const SEvent &event);
			//! Renders the node.
			virtual void render();

			//! Returns the name of the node.
			virtual const c8 *getName() const;

			//! Sets the name of the node.
			virtual void setName(const c8 *name);
			virtual void setName(const core::stringc &name);

			//! Get the axis aligned, not transformed bounding box of this node.
			const core::aabbox3d<f32> &getBoundingBox() const;

			//! Get the axis aligned, transformed and animated absolute bounding box.
			const core::aabbox3d<f32> getTransformedBoundingBox() const;

			//! Get the absolute transformation of the node.
			virtual const core::matrix4 &getAbsoluteTransformation() const;

			//! Returns the relative transformation of the scene node.
			virtual core::matrix4 getRelativeTransformation() const;

			//! Returns whether the node should be visible.
			virtual bool isVisible() const;

			//! Check whether the node is truly visible.
			virtual bool isTrulyVisible() const;

			//! Sets if the node should be visible or not.
			virtual void setVisible(bool isVisible);

			virtual void setStaticNode(bool s);
			virtual bool isStaticNode();

			//! Get the id of the scene node.
			virtual s32 getID() const;

			//! Sets the id of the scene node.
			virtual void setID(s32 id);

			//! Adds a child to this scene node.
			virtual void addChild(ISceneNode *child);

			//! Removes a child from this scene node.
			virtual bool removeChild(ISceneNode *child);

			//! Removes all children of this scene node
			virtual void removeAll();

			//! Removes this scene node from the scene
			virtual void remove();

			//! Gets the scale of the scene node relative to its parent.
			virtual const core::vector3df &getScale() const;

			//! Sets the relative scale of the scene node.
			virtual void setScale(const core::vector3df &scale);

			//! Gets the rotation of the node relative to its parent.
			virtual const core::vector3df &getRotation() const;

			//! Sets the rotation of the node relative to its parent.
			virtual void setRotation(const core::vector3df &rotation);

			//! Gets the position of the node relative to its parent.
			virtual const core::vector3df &getPosition() const;

			//! Sets the position of the node relative to its parent.
			virtual void setPosition(const core::vector3df &newpos);

			//! Gets the absolute position of the node in world coordinates.
			virtual core::vector3df getAbsolutePosition() const;

			//! Enables or disables automatic culling based on the bounding box.
			void setAutomaticCulling(u32 state);

			//! Gets the automatic culling state.
			u32 getAutomaticCulling() const;

			//! Sets if debug data like bounding boxes should be drawn.
			virtual void setDebugDataVisible(u32 state);

			//! Returns if debug data like bounding boxes are drawn.
			u32 isDebugDataVisible() const;

			//! Sets if this scene node is a debug object.
			void setIsDebugObject(bool debugObject);

			//! Returns if this scene node is a debug object.
			bool isDebugObject() const;

			//! Returns a const reference to the list of all children.
			const core::list<ISceneNode *> &getChildren() const;

			//! Changes the parent of the scene node.
			virtual void setParent(ISceneNode *newParent);

			//! Returns the triangle selector attached to this scene node.
			virtual ITriangleSelector *getTriangleSelector() const;

			//! Sets the triangle selector of the scene node.
			virtual void setTriangleSelector(ITriangleSelector *selector);

			//! Updates the absolute position based on the relative and the parents position.
			virtual void updateAbsolutePosition();

			//! Returns the parent of this scene node
			scene::ISceneNode *getParent() const;

			//! Returns type of the scene node
			virtual ESCENE_NODE_TYPE getType() const;

			//! Creates a clone of this scene node and its children.
			virtual ISceneNode *clone(ISceneNode *newParent = 0, ISceneManager *newManager = 0);

			//! Retrieve the scene manager for this node.
			virtual ISceneManager *getSceneManager(void) const;

			template <typename T, typename... Args>
			T *addComponent(Args &&...args)
			{
				T *component = new T(std::forward<Args>(args)...);
				component->owner = this;
				component->OnReady();
				const char *typeName = typeid(T).name();
				auto *node = components.find(typeName);
				if (node)
				{
					IComponent *old = node->getValue();
					if (old && old != component)
					{
						old->OnDestroy();
						old->owner = nullptr;
						delete old;
					}

					node->setValue(component);
				}
				else
				{
					components.insert(typeName, component);
				}

				return component;
			}

			template <typename T>
			bool containsComponent() const
			{
				irr::core::stringc type = typeid(T).name();
				auto *node = components.find(type);
				return node != nullptr;
			}

			template <typename T>
			T *getComponent() const
			{
				irr::core::stringc type = typeid(T).name();

				auto *node = components.find(type);
				if (node)
				{
					return static_cast<T *>(node->getValue());
				}
				return nullptr;
			}

			template <typename T>
			bool removeComponent()
			{
				const char *typeName = typeid(T).name();
				auto *node = components.find(typeName);
				if (node)
				{
					IComponent *component = node->getValue();
					if (component)
					{
						component->OnDestroy();
						component->owner = nullptr;
						delete component;
					}
					components.remove(typeName);
					return true;
				}
				return false;
			}

			virtual void setUseQuaternionRotation(bool use);
			virtual bool isUsingQuaternionRotation() const;

			virtual void setOrientation(const core::quaternion &quat);

			virtual core::quaternion getOrientation() const;
			virtual void rotate(const core::quaternion &quat);

			virtual core::vector3df getForwardVector() const;
			virtual core::vector3df getRightVector() const;
			virtual const core::vector3df getUpVector() const;
			void lookAt(const core::vector3df &target, const core::vector3df &up = core::vector3df(0, 1, 0));
			virtual void lookAt(ISceneNode *target, const core::vector3df &up = core::vector3df(0, 1, 0));

			virtual void moveForward(f32 distance);
			virtual void moveRight(f32 distance);
			virtual void moveUp(f32 distance);
			virtual void moveLocal(const core::vector3df &localDirection);

			virtual void yaw(f32 degrees);
			virtual void pitch(f32 degrees);
			virtual void roll(f32 degrees);
			virtual void rotatePitchYawRoll(f32 pitch, f32 yaw, f32 roll);

			virtual void rotateAroundLocalAxis(const core::vector3df &axis, f32 degrees);
			virtual void moveTowards(const core::vector3df &target, f32 distance);
			virtual void rotateTowards(const core::vector3df &target, f32 maxDegrees);
			virtual void slerpOrientation(const core::quaternion &target, f32 factor);
			virtual void orbitAround(const core::vector3df &center, f32 yawDegrees, f32 pitchDegrees);
			virtual core::vector3df transformVectorByOrientation(const core::vector3df &vec) const;
			virtual core::vector3df transformPoint(const core::vector3df &localPoint) const;
			virtual core::vector3df inverseTransformPoint(const core::vector3df &worldPoint) const;
			virtual core::vector3df transformDirection(const core::vector3df &localDir) const;
			virtual core::vector3df inverseTransformDirection(const core::vector3df &worldDir) const;
			virtual core::vector3df transformVector(const core::vector3df &localVec) const;
			virtual core::vector3df inverseTransformVector(const core::vector3df &worldVec) const;

			virtual f32 getDistanceTo(ISceneNode *other) const;
			virtual f32 getAngleTo(ISceneNode *other) const;

			virtual f32 getYaw() const;
			virtual f32 getPitch() const;
			virtual f32 getRoll() const;
			virtual core::vector3df getEulerAngles() const;
			virtual core::vector3df getEulerAnglesRadians() const;

			virtual bool isLookingAt(const core::vector3df &target, f32 tolerance = 5.0f) const;
			virtual bool isInFieldOfView(ISceneNode *other, f32 fovDegrees = 90.0f) const;
			virtual core::vector3df getDirectionTo(const core::vector3df &target) const;
			virtual core::vector3df getDirectionTo(ISceneNode *other) const;
			virtual core::vector3df getVelocityToReach(const core::vector3df &target, f32 time) const;
			virtual void lerpPosition(const core::vector3df &target, f32 factor);
			virtual void lerpRotation(const core::vector3df &targetRotation, f32 factor);
			virtual void lerpScale(const core::vector3df &targetScale, f32 factor);
			virtual void lerpTransform(ISceneNode *target, f32 factor);
			virtual void alignToNormal(const core::vector3df &normal, const core::vector3df &up = core::vector3df(0, 1, 0));
			virtual void alignUpVector(const core::vector3df &worldUp);
			virtual void resetRotation();
			virtual void resetOrientation();

			virtual bool isNearPosition(const core::vector3df &target, f32 tolerance = 0.1f) const;
			virtual bool isNearRotation(const core::vector3df &targetRotation, f32 tolerance = 1.0f) const;
			virtual core::vector3df getAngularVelocityToReach(const core::vector3df &targetRotation, f32 time) const;
			virtual core::vector3df localToWorld(const core::vector3df &localPos) const;
			virtual core::vector3df worldToLocal(const core::vector3df &worldPos) const;
			virtual core::vector3df localToWorldDirection(const core::vector3df &localDir) const;
			virtual core::vector3df worldToLocalDirection(const core::vector3df &worldDir) const;

			virtual core::vector3df smoothDamp(
				const core::vector3df &current,
				const core::vector3df &target,
				core::vector3df &currentVelocity,
				f32 smoothTime,
				f32 maxSpeed,
				f32 deltaTime) const;

			virtual void smoothDampPosition(
				const core::vector3df &target,
				f32 smoothTime = 0.3f,
				f32 maxSpeed = 100000.0f,
				f32 deltaTime = 0.016f);

			virtual core::vector3df moveTowardsSmooth(
				const core::vector3df &current,
				const core::vector3df &target,
				f32 &currentSpeed,
				f32 maxSpeed,
				f32 acceleration,
				f32 deceleration,
				f32 deltaTime) const;

			virtual void moveTowardsWithAcceleration(
				const core::vector3df &target,
				f32 maxSpeed = 10.0f,
				f32 acceleration = 5.0f,
				f32 deceleration = 10.0f,
				f32 deltaTime = 0.016f);

			virtual core::quaternion rotateTowardsQuat(
				const core::quaternion &from,
				const core::quaternion &to,
				f32 maxDegreesPerSecond,
				f32 deltaTime) const;

			virtual void rotateTowardsDirection(
				const core::vector3df &targetDirection,
				f32 maxDegreesPerSecond = 180.0f,
				f32 deltaTime = 0.016f,
				const core::vector3df &up = core::vector3df(0, 1, 0));

			virtual void rotateTowardsNode(
				ISceneNode *target,
				f32 maxDegreesPerSecond = 180.0f,
				f32 deltaTime = 0.016f,
				const core::vector3df &up = core::vector3df(0, 1, 0));

			virtual void rotateTowardsEuler(
				const core::vector3df &targetRotation,
				f32 maxDegreesPerSecond = 180.0f,
				f32 deltaTime = 0.016f);

			virtual void smoothLookAt(
				const core::vector3df &target,
				f32 rotationSpeed = 90.0f,
				f32 deltaTime = 0.016f,
				const core::vector3df &up = core::vector3df(0, 1, 0));

			virtual void smoothLookAtNode(
				ISceneNode *target,
				f32 rotationSpeed = 90.0f,
				f32 deltaTime = 0.016f,
				const core::vector3df &up = core::vector3df(0, 1, 0));

		protected:
			void cloneMembers(ISceneNode *toCopyFrom, ISceneManager *newManager);

			void setSceneManager(ISceneManager *newManager);
			irr::core::aabbox3d<irr::f32> Box;
			core::map<core::stringc, IComponent *> components;
			core::stringc Name;
			core::matrix4 AbsoluteTransformation;
			core::vector3df RelativeTranslation;
			core::vector3df RelativeRotation;
			core::vector3df RelativeScale;
			core::quaternion RelativeOrientationQuaternion;
			bool UseQuaternionRotation;
			ISceneNode *Parent;
			core::list<ISceneNode *> Children;
			ISceneManager *SceneManager;
			ITriangleSelector *TriangleSelector;
			s32 ID;
			u32 AutomaticCullingState;
			u32 DebugDataVisible;
			ESCENE_NODE_TYPE Type;
			bool IsVisible;
			bool IsStaticObject;
			bool IsDebugObject;

			friend class MeshComponent;
		};

	} // end namespace scene
} // end namespace irr

#endif
