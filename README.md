# Example Projects for Redpoint Plugins

This repository contains all the example projects for all Redpoint Games plugins. Click through to the README for the particular example you are interested in:

- C++ example projects:
  - [Matchmaking](./MM_SimpleCPP/)
  - [Redpoint's EOS Online Subsystem](./EOS_CPlusPlus/)
  - [clang-tidy for Unreal Engine](./ClangTidy/)
- Blueprint example projects:
  - [Matchmaking](./MM_SimpleBP/)
  - [Online Subsystem Blueprints for Redpoint's EOS Online Subsystem](./OSB_RedpointEOS/)
  - [Online Subsystem Blueprints for Epic's EOS Online Subsystem](./OSB_EpicEOS/)
  - [Online Subsystem Blueprints for Steam Subsystem](./OSB_Steam/)
  - [Online Subsystem Blueprints for Null Subsystem](./OSB_Null/)
  - [Online Subsystem Blueprints, authenticating with an external login UI](./OSB_LoginUI/) for subsystems such as Google
  - [Game Management Framework](./GMF/)
  - [Replicated Sublevel Instances](./RSI/)

The example projects currently target **Unreal Engine 5.2**. For previous engine versions, refer to the following Git tags:

- [5.1](https://src.redpoint.games/redpointgames/examples/-/tree/5.1)
- [5.0](https://src.redpoint.games/redpointgames/examples/-/tree/5.0)

Examples for previous engine versions are no longer being maintained. The Agones documentation that was previously in this repository has been removed, as it is superseded by [the dedicated server documentation](https://docs.redpoint.games/eos-online-subsystem/docs/dedis_overview).

## Downloading the examples

There are two ways to download the example projects, either by downloading the ZIPs or by using `git clone`.

You can [download ZIPs of the example projects from the release page](https://src.redpoint.games/redpointgames/examples/-/releases).

Alternatively, to clone this repository with Git, use:

```
git clone --recursive https://src.redpoint.games/redpointgames/examples
```

## Further documentation

For more information on the various example projects, please refer to the relevant documentation:

- [Matchmaking](https://docs.redpoint.games/matchmaking/docs/example_project)
- [EOS Online Subsystem](https://docs.redpoint.games/eos-online-subsystem/docs/example_project)
- [Online Subsystem Blueprints](https://docs.redpoint.games/online-subsystem-blueprints/docs/example_project)
- [clang-tidy for Unreal Engine](https://docs.redpoint.games/clang-tidy-for-unreal-engine/docs/)
- [Game Management Framework](https://docs.redpoint.games/game-management-framework/docs/)
- [Replicated Sublevel Instances](https://www.unrealengine.com/marketplace/en-US/product/replicated-sublevel-instances)