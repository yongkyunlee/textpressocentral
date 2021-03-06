/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Session.h"

#include "MyAuthService.h"
#include "Wt/Auth/HashFunction"
#include "Wt/Auth/PasswordService"
#include "Wt/Auth/PasswordStrengthValidator"
#include "Wt/Auth/PasswordVerifier"
#include "Wt/Auth/Dbo/AuthInfo"
#include "Wt/Auth/Dbo/UserDatabase"

namespace {

    MyAuthService myAuthService;
    Wt::Auth::PasswordService myPasswordService(myAuthService);
}

void Session::configureAuth() {
    myAuthService.setAuthTokensEnabled(true, "logincookie");
    myAuthService.setEmailVerificationEnabled(true);
    myAuthService.setAuthTokenValidity(40320);
    Wt::Auth::PasswordVerifier * verifier = new Wt::Auth::PasswordVerifier();
    verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
    myPasswordService.setVerifier(verifier);
    myPasswordService.setAttemptThrottlingEnabled(true);
    myPasswordService.setStrengthValidator
            (new Wt::Auth::PasswordStrengthValidator());
}

Session::Session(const std::string & Db)
: connection_(Db) {
    connection_.setProperty("show-queries", "true");
    setConnection(connection_);
    mapClass<User > ("tpcuser");
    mapClass<AuthInfo > ("auth_info");
    mapClass<AuthInfo::AuthIdentityType > ("auth_identity");
    mapClass<AuthInfo::AuthTokenType > ("auth_token");
    try {
        createTables();
        std::cerr << "Created database." << std::endl;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "Using existing database";
    }
    users_ = new UserDatabase(*this);
}

Session::~Session() {
    delete users_;
}

Wt::Auth::AbstractUserDatabase& Session::users() {
    return * users_;
}

dbo::ptr<User> Session::user() {
    if (login_.loggedIn())
        return user(login_.user());
    else
        return dbo::ptr<User > ();
}

dbo::ptr<User> Session::user(const Wt::Auth::User & authUser) {
    dbo::ptr<AuthInfo> authInfo = users_->find(authUser);
    dbo::ptr<User> user = authInfo->user();
    if (!user) {
        user = add(new User());
        authInfo.modify()->setUser(user);
    }
    return user;
}

const Wt::Auth::AuthService& Session::auth() {
    return myAuthService;
}

const Wt::Auth::PasswordService & Session::passwordAuth() {
    return myPasswordService;
}
