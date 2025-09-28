#include "CShader.h"

irr::video::CShader::CShader(const core::stringc &name )
{
    Name = name;
 
 
    MaterialType = -1;
 
}

irr::video::CShader::~CShader()
{
}

const irr::core::stringc &irr::video::CShader::getName() const
{
    return Name;
}

irr::s32 irr::video::CShader::getMaterialType() const
{
    return MaterialType;
}
 