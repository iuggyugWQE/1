# Online Subsystem Blueprints Login UI Example

This is a minimal example project showing you how you can use the new login flow handler interfaces to perform authentication with Google. Note that it *does not* include the code to run a HTTP server, as would be required in a real scenario if you were logging in on Windows, because that is outside the scope of the plugin.

You will need to have the Online Subsystem Blueprints plugin installed to use this example project.

Roughly to use the login flow handler interfaces:

- Implement the `ExternalUIFlowHandler` interface on a blueprint.
- Use `Online External UI Subsystem -> Add Login Flow Handler` to register the handler instance.
- Call `Online Identity Subsystem -> Login`, this will call into the external UI subsystem if the OSS needs to, which will then call `OnLoginFlowUIRequired` in your handler.
- The `RequestedURL` needs to be opened in a web browser. At this point you should also be running a HTTP server on `localhost:9001` (at least in the Google subsystem) so you can receive the redirect.
- When the URL comes back (in the demo this is manually inputted), call `Online External UI Subsystem -> NotifyLoginRedirectURL` using the Request ID you got in the original `OnLoginFlowUIRequired` call and pass in the full URL that you received on your HTTP server.
- Get the result from `NotifyLoginRedirectURL` and pass it into `Online External UI Subsystem -> LoginFlowComplete`.

## License

This example code is licensed under the MIT license. The MIT license *DOES NOT* cover any plugin code; only the example project.