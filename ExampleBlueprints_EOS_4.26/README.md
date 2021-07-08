## EOS Online Subsystem Blueprints Example for Unreal Engine 4.26

This is the blueprints example project for Unreal Engine 4.26. It demonstrates how to use [the EOS Online Subsystem plugin](https://redpointgames.gitlab.io/eos-online-subsystem/) and [the Online Subsystem Blueprints plugin](https://redpointgames.gitlab.io/online-subsystem-blueprints/) in your Unreal Engine project.

Although this example comes preconfigured for EOS, you can use it with any online subsystem if you're just looking for a standalone example of Online Subsystem Blueprints. Just skip the steps pertaining to EOS setup and configuration, and switch the `DefaultPlatformService=RedpointEOS` value in `DefaultEngine.ini` to your preferred online subsystem.

### Getting started

This example project requires at least EOS Online Subsystem 2021.07.06 and Online Subsystem Blueprints 2021.07.07. Please make sure you have the latest versions installed through the Epic Games Launcher, or the example project won't work.

### Configure EOS

You will need to configure the EOS plugin with your own organisation, sandbox, product and client IDs, as outlined in [the documentation](https://redpointgames.gitlab.io/eos-online-subsystem/docs/core_configuration).

Once you've replaced the existing values with your own, you should be able to run the example project in the editor.
