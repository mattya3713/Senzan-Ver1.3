# Copilot Instructions

## Project Guidelines
- User prefers using Visual Studio only and has downloaded Assimp via NuGet.
- Follow the coding style defined in `AssimpFBXViewer/Doc/CODING_STYLE.md`.
- Reorganize the implementation order in `FBXModel.cpp` to match the declaration order in `FBXModel.h`:
  - Lifecycle (constructor/destructor/Load/Update/Render/Release)
  - Public accessors (RayCast, Animation-related, Bone-related, Mesh-related)
  - Private helpers

## Key Naming Conventions (Quick Reference)
- Member variables: `m_PascalCase` (e.g., `m_IndexCount`)
- Raw pointers: `m_pPascalCase` (e.g., `m_pDevice`)
- unique_ptr: `m_upPascalCase` (e.g., `m_upBuffer`)
- shared_ptr: `m_spPascalCase` (e.g., `m_spTexture`)
- weak_ptr: `m_wpPascalCase` (e.g., `m_wpParent`)
- static: `s_PascalCase` (e.g., `s_InstanceCount`)
- bool member: `m_IsPascalCase` (e.g., `m_IsActive`)
- Function arguments: `PascalCase` (e.g., `IndexCount`) - **Use PascalCase for function arguments instead of camelCase.**
- Local variables: `snake_case` (e.g., `vertex_count`)
- DirectX::XMVECTOR locals: `v_snake_case` (e.g., `v_position`) - **Local DirectX::XMVECTOR variables use snake_case with v_ prefix (e.g., `v_to_origin`).**
- Classes/Functions: `PascalCase`
- Constants (.cpp): `ALL_CAPS` (e.g., `MAX_COUNT`)
- Global: `g_PascalCase` (e.g., `g_Instance`)
- **NEVER use `using namespace` (including `DirectX`)** - Always explicitly use `DirectX::` for clarity.

## Language Preference
- User prefers responses in Japanese。