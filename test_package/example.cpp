
#include <Information_Model/ElementMock.hpp>
#include <Information_Model/MockBuilder.hpp>
#include <iostream>

int main() {
  Information_Model::BuildInfo info;
  Information_Model::DeviceBuilderPtr builder;
  Information_Model::DevicePtr device;
  Information_Model::ElementPtr element;
  Information_Model::ElementType element_type;
  Information_Model::ElementFunction element_function;
  Information_Model::Parameters parameters;
  Information_Model::ParameterTypes supported_parameters;
  Information_Model::DataVariant value;
  Information_Model::testing::FullMetaInfo meta_info;
  Information_Model::testing::MockBuilderPtr mock_builder;
  Information_Model::testing::DeviceMockPtr device_mock;
  Information_Model::testing::ElementMockPtr element_mock;
  Information_Model::testing::ExecutorPtr executor;

  std::cout << "Integration test successful." << std::endl;

  return 0;
}