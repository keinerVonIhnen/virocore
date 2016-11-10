//
// Created by Raj Advani on 11/9/16.
//

#include "Viro.h"
#include "VROSampleRenderer.h"
#include "VRORenderer.h"
#include "VRODriverOpenGLAndroid.h"
#include "VROImageAndroid.h"
#include "VROSceneRendererCardboard.h"

VROSampleRenderer::VROSampleRenderer(std::shared_ptr<VRORenderer> renderer, VROSceneRendererCardboard *sceneRenderer) :
    _sceneRenderer(sceneRenderer) {

    _sceneController = std::make_shared<VROSceneController>(renderer->getReticle(), renderer->getFrameSynchronizer());

    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    scene->setBackgroundCube(getNiagaraTexture());

    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});

    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.4, 0.4, 0.4 });

    std::shared_ptr<VROLight> spotRed = std::make_shared<VROLight>(VROLightType::Spot);
    spotRed->setColor({ 1.0, 0.0, 0.0 });
    spotRed->setPosition( { -5, 0, 0 });
    spotRed->setDirection( { 1.0, 0, -1.0 });
    spotRed->setAttenuationStartDistance(20);
    spotRed->setAttenuationEndDistance(30);
    spotRed->setSpotInnerAngle(2.5);
    spotRed->setSpotOuterAngle(5.0);

    std::shared_ptr<VROLight> spotBlue = std::make_shared<VROLight>(VROLightType::Spot);
    spotBlue->setColor({ 0.0, 0.0, 1.0 });
    spotBlue->setPosition( { 5, 0, 0 });
    spotBlue->setDirection( { -1.0, 0, -1.0 });
    spotBlue->setAttenuationStartDistance(20);
    spotBlue->setAttenuationEndDistance(30);
    spotBlue->setSpotInnerAngle(2.5);
    spotBlue->setSpotOuterAngle(5.0);

    rootNode->addLight(ambient);
    rootNode->addLight(spotRed);
    rootNode->addLight(spotBlue);

    scene->addNode(rootNode);

    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(2, 4, 2);
    box->setName("Box 1");

    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Phong);
    material->getDiffuse().setContents(std::make_shared<VROTexture>(std::make_shared<VROImageAndroid>("boba.png")));
    material->getSpecular().setContents(std::make_shared<VROTexture>(std::make_shared<VROImageAndroid>("specular.png")));

    std::vector<std::string> modifierCode =  { "uniform float testA;",
                                               "uniform float testB;",
                                               "_geometry.position.x = _geometry.position.x + testA;"
    };
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                                      modifierCode);

    modifier->setUniformBinder("testA", [](VROUniform *uniform, GLuint location) {
        uniform->setFloat(1.0);
    });
    material->addShaderModifier(modifier);

    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition({0, 0, -5});

    rootNode->addChildNode(boxNode);
    //boxNode->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));

    /*
     Create a second box node behind the first.
     */
    std::shared_ptr<VROBox> box2 = VROBox::createBox(2, 4, 2);
    box2->setName("Box 2");

    std::shared_ptr<VROMaterial> material2 = box2->getMaterials()[0];
    material2->setLightingModel(VROLightingModel::Phong);
    material2->getDiffuse().setContents(std::make_shared<VROTexture>(std::make_shared<VROImageAndroid>("boba.png")));
    material2->getSpecular().setContents(std::make_shared<VROTexture>(std::make_shared<VROImageAndroid>("specular.png")));

    std::shared_ptr<VRONode> boxNode2 = std::make_shared<VRONode>();
    boxNode2->setGeometry(box2);
    boxNode2->setPosition({0, 0, -9});
    boxNode2->addLight(ambient);


    rootNode->addChildNode(boxNode2);

    /*
     Create a second box node behind the first.
     */
    std::shared_ptr<VROBox> box3 = VROBox::createBox(2, 4, 2);
    box3->setName("Box 3");

    std::shared_ptr<VROMaterial> material3 = box3->getMaterials()[0];
    material3->setLightingModel(VROLightingModel::Phong);
    material3->getDiffuse().setContents(std::make_shared<VROTexture>(std::make_shared<VROImageAndroid>("boba.png")));
    material3->getSpecular().setContents(std::make_shared<VROTexture>(std::make_shared<VROImageAndroid>("specular.png")));

    std::shared_ptr<VRONode> boxNode3 = std::make_shared<VRONode>();
    boxNode3->setGeometry(box3);
    boxNode3->setPosition({0, 0, -13});

    rootNode->addChildNode(boxNode3);

    //[self.view setCameraRotationType:VROCameraRotationType::Orbit];
    //[self.view setOrbitFocalPoint:boxNode->getPosition()];

    //dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(6);

        spotRed->setPosition({5, 0, 0});
        spotRed->setDirection({-1, 0, -1});

        spotBlue->setPosition({-5, 0, 0});
        spotBlue->setDirection({1, 0, -1});

        VROTransaction::commit();
    //});
}

VROSampleRenderer::~VROSampleRenderer() {

}

std::shared_ptr<VROTexture> VROSampleRenderer::getNiagaraTexture() {
std::vector<std::shared_ptr<VROImage>> cubeImages =  {
        std::make_shared<VROImageAndroid>("px.png"),
        std::make_shared<VROImageAndroid>("nx.png"),
        std::make_shared<VROImageAndroid>("py.png"),
        std::make_shared<VROImageAndroid>("ny.png"),
        std::make_shared<VROImageAndroid>("pz.png"),
        std::make_shared<VROImageAndroid>("nz.png")
};

return std::make_shared<VROTexture>(cubeImages);
}

void VROSampleRenderer::setupRendererWithDriver(VRODriver *driver) {
    _driver = driver;
    _sceneRenderer->setSceneController(_sceneController);
}