## Online Subsystem for Epic's EOS 4.27 plugin

This is a version of the blueprints example project targeting Epic's EOS plugin that ships with Unreal Engine 4.27.

### IMPORTANT COMPATIBILITY NOTES: PLEASE READ

Epic's EOS plugin **is still heavily experimental** in Unreal Engine 4.27, and there are plenty of rough edges. We can't fix bugs inside Epic's code, because that code is inside Unreal Engine itself. A non-exhaustive list of issues and other notes we've found with Epic's plugin at the time of writing this document:

- Play-in-editor does not automatically use the Developer Authentication Tool, and can't use different credentials per play-in-editor instance (at least not easily from blueprints). This means you can't easily test multiplayer in the editor from blueprints over e.g. EOS P2P.
- `AutoLogin` only works for Epic Games Launcher authentication. You'll need to use `Login` and use `accountportal` as the credential type, and enable EAS in the options.
- Alternatively, you can use turn on EOS authentication (I think it's "Use Crossplatform User IDs") and authenticate with Steam. This requires setting Steam as the native platform subsystem and Epic's EOS as the default subsystem. It will also only work when launching the game as standalone; you won't be able to authenticate in the editor with this setup.
- Reading stats doesn't work. Epic's C++ code implementation fails with `EOS_InvalidUser`, despite the call providing a valid local user ID.
- Reading global leaderboards doesn't work. Epic's C++ code implementation fails with `EOS_InvalidUser`, despite the call providing a valid local user ID.
- Reading friend leaderboards just stalls forever if you don't have any friends on the friends list, due to a bug in Epic's C++ code.
- Reading friend leaderboards fails with `EOS_InvalidUser` if you do have friends, despite the call providing a valid local user ID.
- Changing Project Settings doesn't take effect until you restart the editor.
- Voice chat just doesn't seem to work at all, as it doesn't properly authenticate with the EOS backend.
- Session IDs appear as "0000.." in the example, although sessions do appear to be created on the backend properly.

A list of things that we've confirmed work:

- Reading the friends list, at least for Epic Games friends.
- Reading achievements.
- Getting a list of files in Title Storage.
- Downloading files from Title Storage.
- Reading, writing and listing files in Player Data Storage (User Cloud).
- Setting user presence for Epic Games accounts, although it does log a useless error message to the console.

We want to be clear: the issues listed above **are not issues in Online Subsystem Blueprints**. They are issues in Epic's implementation because it is experimental.

Your options when using Online Subsystem Blueprints with EOS are as follows:

- You can use Epic's plugin and workaround the issues above by limiting the EOS features you're using, or
- You can use Redpoint's plugin, which is well supported and doesn't have the above issues.

### Configure EOS

This example project is currently configured with the example client IDs. You'll need to update the Project Settings to use your own organisation, sandbox, product and client IDs.

Once you've replaced the existing values with your own, you should be able to login in the editor and sign in with your Epic Games account in your web browser.
