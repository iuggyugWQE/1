-- Manually validated which bugprone- checks apply to Unreal Engine code.
enable_checks("bugprone-bool-pointer-implicit-conversion")
enable_checks("bugprone-branch-clone")
enable_checks("bugprone-copy-constructor-init")
enable_checks("bugprone-forwarding-reference-overload")
enable_checks("bugprone-implicit-widening-of-multiplication-result")
enable_checks("bugprone-incorrect-roundings")
enable_checks("bugprone-infinite-loop")
enable_checks("bugprone-integer-division")
-- Too many warnings in test module, and we don't have NOLINTBEGIN/NOLINTEND until clang-tidy 14.
-- enable_checks("bugprone-macro-parentheses")
enable_checks("bugprone-macro-repeated-side-effects")
enable_checks("bugprone-misplaced-widening-cast")
enable_checks("bugprone-parent-virtual-call")
enable_checks("bugprone-redundant-branch-condition")
enable_checks("bugprone-signed-char-misuse")
enable_checks("bugprone-sizeof-expression")
enable_checks("bugprone-suspicious-enum-usage")
enable_checks("bugprone-suspicious-semicolon")
enable_checks("bugprone-swapped-arguments")
enable_checks("bugprone-terminating-continue")
enable_checks("bugprone-too-small-loop-variable")
enable_checks("bugprone-undelegated-constructor")
enable_checks("bugprone-unhandled-self-assignment")
enable_checks("bugprone-virtual-near-miss") 

-- Manually validated which performance- checks apply to Unreal Engine code.
enable_checks("performance-for-range-copy")
enable_checks("performance-implicit-conversion-in-loop")
enable_checks("performance-move-constructor-init")
enable_checks("performance-no-automatic-move")
enable_checks("performance-trivially-destructible")
enable_checks("performance-unnecessary-copy-initialization")
enable_checks("performance-unnecessary-value-param")

-- Unreal Engine checks.
enable_checks("unreal-*")

-- Treat certain bugs as errors.
treat_as_errors("bugprone-*")
treat_as_errors("performance-*") 
treat_as_errors("unreal-*")
treat_as_errors("eos-*")

check {
    name = "eos-uninitialized-struct",
    enabled = true,
    description = [[
        Detects uninitialized EOS variables. These are dangerous because if new fields
        are added in later SDK versions, they will be left uninitialized in Shipping builds.

        All EOS structs are required to be initialized like so:
        
        EOS_SomeStruct Val = {};

        With all field assignments following after that.
    ]],
    matcher = [[
        varDecl(hasType(namedDecl(matchesName("^EOS_.+")).bind("eos_type")), hasLocalStorage(), hasDescendant(cxxConstructExpr(argumentCountIs(0)))).bind("variable_declaration")
    ]],
    message = [[
        uninitialized EOS structure will lead to undefined behaviour at runtime; use an empty initializer to force zero initialization
    ]],
    callsite = "variable_declaration"
}

check {
    name = "unreal-shared-class-is-copyable",
    description = [[
        Detects if a class implements TSharedFromThis<>, but doesn't use UE_NONCOPYABLE
        to prevent implicit copies.

        If you don't delete the constructors with UE_NONCOPYABLE, C++ will generate default
        copy and move constructors which will allow broken code like this to compile:

        TSomeClass& A = *SomePtrToSharedClassA;
        TSomeClass B = A; // Implicit copy.
        B.AsShared();     // Crash.
    ]],
    matcher = [[
        cxxRecordDecl(
            isDerivedFrom(
                classTemplateSpecializationDecl(
                    hasName("TSharedFromThis")
                )
            ),
            hasDefinition(),
            has(decl()),
            unless(
                has(
                    cxxConstructorDecl(
                        isCopyConstructor(), 
                        isDeleted(),
                        isExpandedFromMacro("UE_NONCOPYABLE")
                    )
                )
            ),
            unless(
                anyOf(
                    isExpandedFromMacro("DEFINE_LATENT_AUTOMATION_COMMAND"),
                    isExpandedFromMacro("DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER"),
                    isExpandedFromMacro("DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER"),
                    isExpandedFromMacro("DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER"),
                    isExpandedFromMacro("DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER"),
                    isExpandedFromMacro("DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER")
                )
            )
        ).bind("class_definition")
    ]],
    message = [[class inherits from TSharedFromThis<> (either directly or otherwise), but does not delete the copy or constructors. use UE_NONCOPYABLE(ClassName) in the class body to delete the copy and move constructors.]],
    callsite = "class_definition"
}