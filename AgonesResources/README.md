# Agones resources

This folder contains files and other resources you'll need in order to build a dedicated server from ExampleCPP_EOS_4.26 and have it deployed out to your Kubernetes cluster with Agones installed.

### Building the server

First you'll need to build the dedicated server in Unreal Engine. `BuildServer.ps1` is a script you can use that will build an Unreal Engine project into a dedicated server.

Note that you'll need to point it at your custom build of Unreal Engine, as versions of Unreal Engine installed from the Epic Games Launcher do not include dedicated server support.

You can use the script like so:

```bat
powershell .\AgonesResources\BuildServer.ps1 -FolderName "ExampleCPP_EOS_4.26" -ProjectName "ExampleCPP_EOS_4.26" -EnginePath "C:\Path\To\Your\Engine\Build"
```

If you have Skaffold installed on Windows and are deploying from the same machine you build on, you can also ask the script to deploy it for you:

```bat
powershell .\AgonesResources\BuildServer.ps1 -FolderName "ExampleCPP_EOS_4.26" -ProjectName "ExampleCPP_EOS_4.26" -EnginePath "C:\Path\To\Your\Engine\Build" -Deploy
```

### Deploying the server

You will need to have Skaffold installed on the local machine. You will also need to have the built server. If you are deploying from a different machine than you were building on (e.g. you are deploying from Linux), make sure your CI/CD configuration correctly archives and retrieves the `OutputServer` folder that the binaries were written to.

First, **update the skaffold.yaml and exampleoss-4.26-fleet.yaml files** as per the comments in those files. They need to be updated for where you want to push your game server image to.

Then, run Skaffold:

```sh
skaffold run
```

### Doing it from GitLab CI

You can perform the steps above from GitLab CI, assuming that you have configured and set up your own CI runners. In this case, building is done on a Windows machine and deployment is done from a Linux machine.

This assumes that you are deploying your dedicated server image to GitLab's Docker Registry. If this is the case, make sure you have [configured your Kubernetes cluster to be able to pull from GitLab's Docker Registry](https://chris-vermeulen.com/using-gitlab-registry-with-kubernetes/) and then update the required files as indicated in the "Deploying the server" step.

Set the following into a `.gitlab-ci.yml` file in the root of this repository. If you're working in a different repository, make sure the paths in `skaffold.yaml` and `BuildServer.ps1` have been updated as appropriate.

```yaml
stages:
  - build
  - deploy

build-dedicated-server:
  stage: build
  tags:
    - your-windows-tag
  script:
    - .\AgonesResources\BuildServer.ps1 -FolderName "ExampleCPP_EOS_4.26" -ProjectName "ExampleCPP_EOS_4.26" -EnginePath "C:\Path\To\Your\Engine\Build"
    - Write-Output "Finalising..."
    - exit $LastExitCode
  only:
    - main
  interruptible: true
  artifacts:
    paths:
      - "ExampleCPP_EOS_4.26/OutputServer/LinuxServer"
    untracked: false
    expire_in: 1 hour

deploy-dedicated-server:
  stage: deploy
  needs:
    - build-dedicated-server
  tags:
    - your-linux-tag
  script:
    - docker login -u $CI_REGISTRY_USER -p $CI_REGISTRY_PASSWORD $CI_REGISTRY
    - cd BuildScripts && skaffold run
  only:
    - main
  interruptible: true
```