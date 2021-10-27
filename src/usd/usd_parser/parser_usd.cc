/*
 * Copyright (C) 2021 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <pxr/usd/sdf/reference.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/base/tf/stringUtils.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/primRange.h>
#include "pxr/usd/usdGeom/gprim.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/rotation.h"

#include "pxr/usd/usdPhysics/scene.h"
#include "pxr/usd/usdPhysics/joint.h"
#include "pxr/usd/usdPhysics/fixedJoint.h"

#include "pxr/usd/usdLux/light.h"
#include "pxr/usd/usdLux/sphereLight.h"

// #include "pxr/base/tf/staticTokens.h"
// #include "pxr/usd/usdRi/tokens.h"

#include "usd_parser/parser_usd.hh"
#include "physics.hh"
#include "joints.hh"
#include "links.hh"
#include "sdf/Console.hh"

#include <fstream>

#include <ignition/common/Util.hh>

namespace usd {

ModelInterfaceSharedPtr parseUSD(const std::string &xml_string)
{
  ModelInterfaceSharedPtr model(new ModelInterface);
  model->clear();

  auto referencee = pxr::UsdStage::CreateInMemory();
  if (!referencee->GetRootLayer()->ImportFromString(xml_string))
  {
    return nullptr;
  }

  // Get robot name
  const char *name = "test";
  if (!name)
  {
    model.reset();
    return model;
  }
  model->name_ = std::string(name);

  // TODO(ahcorde): Get all Material elements

  auto range = pxr::UsdPrimRange::Stage(referencee);

  double metersPerUnit;
  referencee->GetMetadata<double>(pxr::TfToken("metersPerUnit"), &metersPerUnit);
  std::cerr << "/* metersPerUnit */" << metersPerUnit << '\n';
  std::string rootPath;
  std::string nameLink;

  // insert <link name="world"/>
  LinkSharedPtr worldLink = nullptr;
  worldLink.reset(new Link);
  worldLink->clear();
  worldLink->name = "world";
  model->links_.insert(make_pair(worldLink->name, worldLink));

  // Get all Link elements
  for (auto const &prim : range ) {

    std::string primName = pxr::TfStringify(prim.GetPath());

    sdferr << "------------------------------------------------------\n";
    sdferr << "pathName " << primName << "\n";
    // sdferr << prim.GetName().GetText() << "\n";

    size_t pos = std::string::npos;
    if ((pos  = primName.find("/World") )!= std::string::npos)
    {
      primName.erase(pos, std::string("/World").length());
    }

    std::vector<std::string> tokens = ignition::common::split(primName, "/");
    if (tokens.size() == 0)
      continue;

    if (tokens.size() == 1)
    {
      rootPath = prim.GetName().GetText();
    }
    std::cerr << "rootPath " << rootPath << " " << tokens.size() << '\n';

    if (tokens.size() > 1)
    {
      nameLink = "/" + tokens[0] + "/" + tokens[1];
    }

    if (prim.IsA<pxr::UsdPhysicsScene>())
    {
      usd::ParsePhysicsScene(prim);
    }

    if (prim.IsA<pxr::UsdPhysicsJoint>())
    {
      sdferr << "UsdPhysicsJoint" << "\n";

      JointSharedPtr joint = usd::ParseJoints(prim, primName, metersPerUnit);
      if (joint != nullptr)
      {
        model->joints_.insert(make_pair(joint->name, joint));
      }

      continue;
    }

    if (tokens.size() == 1)
      continue;

    if (prim.IsA<pxr::UsdLuxLight>())
    {
      sdferr << "Light" << "\n";
      if (prim.IsA<pxr::UsdLuxSphereLight>())
      {
        sdferr << "Sphere light" << "\n";
      }
      continue;
    }

    // if (!prim.IsA<pxr::UsdGeomGprim>())
    // {
    //   sdferr << "Not a geometry" << "\n";
    //   continue;
    // }

    LinkSharedPtr link = nullptr;
    auto it = model->links_.find(nameLink);
    if (it != model->links_.end())
    {
      link = usd::ParseLinks(prim, it->second, metersPerUnit);
    }
    else
    {
      link = usd::ParseLinks(prim, link, metersPerUnit);

      if (link)
      {
          model->links_.insert(make_pair(nameLink, link));
      }
    }

    if (model->links_.empty()){
      model.reset();
      return model;
    }
  }

  // for (auto link: model->links_)
  // {
  //   std::cerr << "link name " << link.second->name << '\n';
  // }
  //
  // for (auto joint: model->joints_)
  // {
  //   std::cerr << "joints name " << joint.second->name << '\n';
  //   std::cerr << "\t parent " << joint.second->parent_link_name << '\n';
  //   std::cerr << "\t child " << joint.second->child_link_name << '\n';
  // }

  // every link has children links and joints, but no parents, so we create a
  // local convenience data structure for keeping child->parent relations
  std::map<std::string, std::string> parent_link_tree;
  parent_link_tree.clear();

  // building tree: name mapping
  try
  {
    model->initTree(parent_link_tree);
  }
  catch(ParseError &e)
  {
    model.reset();
    sdferr << "error initTree " << e.what()  << "\n";
    return model;
  }

  // find the root link
  try
  {
    model->initRoot(parent_link_tree);
  }
  catch(ParseError &e)
  {
    model.reset();
    sdferr << "error initRoot " << e.what() << "\n";
    return model;
  }

  if (referencee)
    return model;
  return nullptr;
}


ModelInterfaceSharedPtr parseUSDFile(const std::string &filename)
{
  auto referencee = pxr::UsdStage::Open(filename);
  std::ifstream stream( filename.c_str() );
  if (!stream)
  {
    return ModelInterfaceSharedPtr();
  }
  std::string xml_str((std::istreambuf_iterator<char>(stream)),
                     std::istreambuf_iterator<char>());
  return usd::parseUSD(xml_str);
}

void exportUSD()
{

}
}