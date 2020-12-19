#include <Catch2/catch_all.hpp>
#include <engine/rendering/vulkan/shader_parsing.h>
#include <engine/rendering/vulkan/fileutils.h>

SCENARIO("All Uniform types are recognized", "[SpirvParser]") {
  SpirvParser parser;

  GIVEN("Vertex shader with all uniform types inside the same set0") {
    auto shader = ReadFile("../data/shaders/spirv/tests/test0_all_uniform_types.vert.spv");

    WHEN("Shader is parsed") {
      auto uniforms = parser.ParseShader(shader);

      /// increment every time there is a new type of uniform is parsed.
      /// exist with reason to not forget to fix the test with new type added.
      auto differentUniformsCount = 1;

      THEN("Every uniform is recognized.") {
        REQUIRE(uniforms.sets.size() == 1);
        REQUIRE(uniforms.sets[0].inUse == true);
        REQUIRE(uniforms.sets[0].bindings.size() == differentUniformsCount);
        REQUIRE(uniforms.sets[0].bindings[0].type == UniformType::UniformBuffer);
        REQUIRE(uniforms.sets[0].bindings[0].stages == SHADER_VERTEX_STAGE);
        REQUIRE(uniforms.uniformsMap.find("ubo_uniform") != uniforms.uniformsMap.end());
      }
    }
  }
}

SCENARIO("Uniforms from different stages can be merged", "[SpirvParser]") {
  SpirvParser parser;

  GIVEN("Vertex and fragment shaders with common uniforms") {
    auto vertex = ReadFile("../data/shaders/spirv/tests/test1_merge_uniforms.vert.spv");
    auto fragment = ReadFile("../data/shaders/spirv/tests/test1_merge_uniforms.frag.spv");

    WHEN("Their uniforms are parsed and merged") {
      auto uniforms = parser.ParseShader(vertex) + parser.ParseShader(fragment);

      THEN("Merged uniforms are consistent") {
        REQUIRE(uniforms.sets.size() == 1);
        REQUIRE(uniforms.sets[0].inUse == true);
        REQUIRE(uniforms.sets[0].bindings.size() == 2);
        REQUIRE(uniforms.sets[0].bindings[0].stages == (SHADER_VERTEX_STAGE | SHADER_FRAGMENT_STAGE));
        REQUIRE(uniforms.sets[0].bindings[1].stages == SHADER_VERTEX_STAGE);
        REQUIRE(uniforms.uniformsMap.find("ubo_uniform") != uniforms.uniformsMap.end());
        REQUIRE(uniforms.uniformsMap.find("single_ubo_uniform") != uniforms.uniformsMap.end());
      }
    }
  }
}

int main(int argc, char* argv[]) {
  // global setup...

  int result = Catch::Session().run(argc, argv);

  // global clean-up...

  return result;
}