//
//  Kamcord.h
//  cocos2d-ios
//
//  Created by Kevin Wang on 5/5/12.
//  Copyright (c) 2012 Kamcord. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#import "KCGLView.h"
#import "CCDirectorIOS.h"

// Convenient for game developers
#import "KamcordMacros.h"
#import "Common/View/KCViewController.h"
#import "Common/Core/Audio/KCAudio.h"
#import "Common/Core/Audio/KCSound.h"

#define KAMCORD_VERSION "0.9.7"

// --------------------------------------------------------
// The following enum and protocol are only relevant
// if you're implementing your own custom UI.
// If you're using the default Kamcord UI, please
// ignore the following as it is irrelevant for you.

@class GTMOAuth2Authentication;

// --------------------------------------------------------
// API elements for custom sharing UI.
// Will be documented soon as we roll out complete
// support for custom UIs.
typedef enum
{
    NO_ERROR,
    FACEBOOK_NOT_AUTHENTICATED,
    FACEBOOK_LOGIN_CANCELLED,
    FACEBOOK_DAILY_SHARE_EXCEEDED,
    FACEBOOK_SERVER_ERROR,
    
    TWITTER_NOT_SETUP,
    TWITTER_NOT_AUTHENTICATED,
    TWITTER_NO_ACCOUNTS,
    TWITTER_SERVER_ERROR,
    
    YOUTUBE_NOT_AUTHENTICATED,
    YOUTUBE_LOGIN_CANCELLED,
    YOUTUBE_SERVER_ERROR,
    
    EMAIL_NOT_SETUP,
    EMAIL_CANCELLED,
    EMAIL_FAILED,
    NO_INTERNET,
    
    KAMCORD_SERVER_ERROR,
    KAMCORD_S3_ERROR,
    
    NOTHING_TO_SHARE,
    MESSAGE_TOO_LONG,
    
    VIDEO_PROCESSING_ERROR,
} KCShareStatus;


@protocol KCShareDelegate <NSObject>

@required
// Only after this callback (or a generalError below) is it safe
// make a new share request.
- (void)shareStartedWithSuccess:(BOOL)success error:(KCShareStatus)error;

// Errors that happen along the way
- (void)generalError:(KCShareStatus)error;



@optional
// --------------------------------------------------------
// Callbacks for case 1 (see below)

// Don't worry about deferring on sharing until these are
// called. Our internal system will wait until conversion
// is finished before your share request is executed.
- (void)videoMergeFinishedWithSuccess:(BOOL)success error:(NSError *)error;
- (void)videoConversionFinishedWithSuccess:(BOOL)success error:(NSError *)error;

// Auth requests
- (void)facebookAuthFinishedWithSuccess:(BOOL)success;
- (void)twitterAuthFinishedWithSuccess:(BOOL)success;
- (void)youTubeAuthFinishedWithSuccess:(BOOL)success;

// Beginning of share process
// First: auth verification
- (void)facebookShareStartedWithSuccess:(BOOL)success error:(KCShareStatus)error;
- (void)twitterShareStartedWithSuccess:(BOOL)success error:(KCShareStatus)error;
- (void)youTubeUploadStartedWithSuccess:(BOOL)success error:(KCShareStatus)error;
- (void)emailSentWithSuccess:(BOOL)success error:(KCShareStatus)error;
//
// End of share process
- (void)facebookShareFinishedWithSuccess:(BOOL)success error:(KCShareStatus)error;
- (void)twitterShareFinishedWithSuccess:(BOOL)success error:(KCShareStatus)error;
- (void)youTubeUploadFinishedWithSuccess:(BOOL)success error:(KCShareStatus)error;


//
// Retrying failed uploads/shares
//


// Indicate that we are queueing a failed share for retrying later
- (void)queueingFailedShareForFutureRetry;

// Indicate that we are retrying failed uploads/shares
- (void)retryingPreviouslyFailedShares:(NSUInteger)numShares;

// Indicate that we have given up on retrying a failed share
- (void)stoppingRetryForFailedShare;


// The following callback will be made for both Option 1 and Option 2:

// We call this after we've received a video URL from the server.
// The purpose of this call is to give you two pieces of information:
//
//   1. The URL the video will be at once the upload finishes
//   2. The URL the video thumbnail will be at once the upload finishes
//
// We also pass in the data dictionary you passed in with the share request,
// along with any possible error messages.
- (void)videoWillBeginUploading:(NSURL *)onlineVideoURL
                      thumbnail:(NSURL *)onlineThumbnailURL
                           data:(NSDictionary *)data
                          error:(NSError *)error;


// If the error object is nil, then the video and thumbnail
// URLs are valid. Otherwise, the video and thumbnail URLs
// should not be considered valid, even if they are non-nil.
// The data is the original object that was passed in with the share request.
- (void)videoIsReadyToShare:(NSURL *)onlineVideoURL
                  thumbnail:(NSURL *)onlineThumbnailURL
                    message:(NSString *)message
                       data:(NSDictionary *)data
                      error:(NSError *)error;
@end


// --------------------------------------------------------
// Callbacks for video playback
// 
@protocol KCMoviePlayerDelegate <NSObject>

// Called when the movie player is presented
- (void)moviePlayerDidAppear;

// Called when the movie player is dismissed
- (void)moviePlayerDidDisappear;

@end



@interface Kamcord : NSObject

////////////////////////////////////////////////
// Public methods

// Setup
+ (void) setDeveloperKey:(NSString *)key
         developerSecret:(NSString *)secret
                 appName:(NSString *)name;
+ (NSString *) developerKey;
+ (NSString *) developerSecret;
+ (NSString *) appName;

// View and OpenGL
+ (void) setParentViewController:(UIViewController *)viewController;
+ (UIViewController *) parentViewController;

+ (void) setView:(KCGLView *)glView;
+ (KCGLView *) openGLView;

// Convenience wrapper around [[UIApplication sharedApplication] statusBarOrientation]
+ (KCDeviceOrientation) deviceOrientation;
+ (BOOL)useUIKitAutorotation;

// Social media
// YouTube
+ (void) setYouTubeTitle:(NSString *)title
             description:(NSString *)description 
                tags:(NSString *)tags;
+ (NSString *) youtubeTitle;
+ (NSString *) youtubeDescription;// Email
+ (NSString *) youtubeTags;

+ (void) setDefaultYouTubeMessage:(NSString *)message;
+ (NSString *)defaultYouTubeMessage;

// Facebook
+ (void) setFacebookTitle:(NSString *)title
                  caption:(NSString *)caption
              description:(NSString *)description;
+ (NSString *) facebookTitle;
+ (NSString *) facebookCaption;
+ (NSString *) facebookDescription;

+ (void) setDefaultFacebookMessage:(NSString *)message;
+ (NSString *)defaultFacebookMessage;

// Twitter
+ (void)setDefaultTweet:(NSString *)tweet;
+ (NSString *)defaultTweet;

// Email
+ (void)setDefaultEmailSubject:(NSString *)subject
                          body:(NSString *)body;
+ (NSString *)defaultEmailSubject;
+ (NSString *)defaultEmailBody;


+ (void) setLevel:(NSString *)level
            score:(NSNumber *)score;
+ (NSString *) level;
+ (NSNumber *) score;

////////////////////
// Video recording
//

// Not necessary to call. However, if you want to avoid
// the slight FPS drop when calling startRecording,
// call this method earlier when there's very little
// processing and a slight drop in FPS won't be noticed
// (for example, on startup, or an end of level screen).
+ (BOOL)prepareNextVideo;

+ (BOOL)startRecording;
+ (BOOL)stopRecording;
+ (BOOL)stopRecordingAndDiscardVideo;
+ (BOOL)resume;
+ (BOOL)pause;

// Displays the Kamcord view inside the previously set parentViewController;
+ (void) showView;

// Video recording settings
// For release, use SMART_VIDEO_DIMENSIONS:
//   iPad 1 and 2: 512x384
//   iPad 3: 1024x768
//   All iPhone and iPods: 480x320
//
// For trailers, use TRAILER_VIDEO_RESOLUTION
//   All iPads: 1024x768
//   iPhone/iPod non-retina: 480x320
//   iPhone/iPad retina: 960x640
typedef enum {
    SMART_VIDEO_RESOLUTION,
    TRAILER_VIDEO_RESOLUTION,
} KC_VIDEO_RESOLUTION;

// Size refers to the pixel dimensions. 
+ (void) setVideoResolution:(KC_VIDEO_RESOLUTION)resolution;
+ (KC_VIDEO_RESOLUTION) videoResolution;


// Audio recording
+ (KCAudio *) playSound:(NSString *)filename
                   loop:(BOOL)loop;
+ (KCAudio *) playSound:(NSString *)filename;

// Will stop all looping, non-looping, or looping and non-looping sounds.
typedef enum
{
    NONLOOPING_SOUNDS,
    LOOPING_SOUNDS,
    ALL_SOUNDS
} KC_SOUND_TYPE;
+ (void)stopAllSounds:(KC_SOUND_TYPE)soundType;

// If you have specific sounds you want to overlay at particular times,
// pass in an array populated with KCSound objects.
+ (BOOL)stopRecordingAndAddSounds:(NSArray *)sounds;


// When the user shares a video, should the Kamcord UI wait for
// the video to finish converting before automatically dismissing
// the share screen?
//
// This can be turned on for games that experience a performance
// hit if the video processing is happening in the background
// while the user is playing the next round or level.
//
// The second argument determines whether or not the video processing
// progress bar is always visible (set to YES), or only visible
// when the user presses a button to share (defaults to this setting, which is NO).
+ (void)setEnableSynchronousConversionUI:(BOOL)on
                   alwaysShowProgressBar:(BOOL)alwaysShow;
+ (BOOL)enableSynchronousConversionUI;
+ (BOOL)alwaysShowProgressBar;


// Show the video player controls when the replay is shown?
// By default YES, since user studies have shown that users
// don't understand what they're seeing is an actual video
// as opposed to the round restarting again.
+ (void)setShowVideoControlsOnReplay:(BOOL)showControls;
+ (BOOL)showVideoControlsOnReplay;


// Every time you call startRecording, Kamcord will delete
// the previous video if it is not currently being shared.
// If you want to erase the video before then, you can call
// this method. If the video is currently being shared, it
// it will be erased after the next share is complete.
//
// Please be careful with this call. If there are no pending shares,
// the video WILL be erased. If, for instance, you call
// [Kamcord presentVideoPlayerInViewController:] and
// then [Kamcord cancelConversionForLatestVideo] while the video is
// being shown, you may get EXC_BAD_ACCESS. 
//

// Returns YES if conversion for the latest video was cancelled.
// Returns NO if the latest video has already been shared and we need to wait for the conversion.
+ (BOOL)cancelConversionForLatestVideo;

// Optional: Set the maximum video time in seconds. If the recorded video goes over that time,
//           then only the last N seconds are taken.
//           To not have a maximum video time, set this value to 0 (the default).
+ (void)setMaximumVideoLength:(NSUInteger)seconds;
+ (NSUInteger)maximumVideoLength;



// --------------------------------------------------------
// Custom sharing API


// Used for both Case 1 and Case 2

// Replay the latest video in the parent view controller.
// The "latest video" is defined as the last one for which
// you called [Kamcord stopRecording].
+ (void)presentVideoPlayerInViewController:(UIViewController *)parentViewController;


// The object that will receive callbacks when the movie player
// is show and dismissed.
+ (void)setMoviePlayerDelegate:(id <KCMoviePlayerDelegate>)delegate;
+ (id <KCMoviePlayerDelegate>)moviePlayerDelegate;


// The object that will receive callbacks about sharing state.
// You must make sure that this object is retained until
// all the callbacks are done. This delegate is retained
// until all the callbacks are complete, after which it
// is released by Kamcord.
+ (void)setShareDelegate:(id <KCShareDelegate>)delegate;
+ (id <KCShareDelegate>)shareDelegate;


// Case 1: Use the following API for sharing if you want
//         your own custom UI but would like Kamcord to handle
//         all of the Facebook/Twitter/YouTube authentication for you.

// Authenticate to the three social media services
+ (void)showFacebookLoginView;
+ (void)showTwitterAuthentication; 
+ (void)showYouTubeLoginViewInViewController:(UIViewController *)parentViewController;

// Status of authentication
+ (BOOL)facebookIsAuthenticated;
+ (BOOL)twitterIsAuthenticated;
+ (BOOL)youTubeIsAuthenticated;

+ (void)performFacebookLogout;
+ (void)performYouTubeLogout;

// The method to share a message on these services.
// You can also use this if you want to mix different
// authentications. For instance, you can handle
// Facebook and Twitter auth and let Kamcord upload
// to YouTube with its own auth (which it got via
// presentYouTubeLoginViewInViewController: above.
// Simply call this with shareFacebook and shareTwitter set to NO
// and shareYouTube set to YES.
//
// Once the video uploads are done, we will call back
// to videoIsReadyToShare.
//
// Returns YES if the share was accepted for processing.
// Returns NO if there was a previous share that is still
// in its early stages (specifically, before a generalError:
// or shareshareStartedWithSuccess:error: callback).
+ (BOOL)shareVideoOnFacebook:(BOOL)shareFacebook
                     Twitter:(BOOL)shareTwitter
                     YouTube:(BOOL)shareYouTube
                 withMessage:(NSString *)message;

// Show the send email dialog with the Kamcord URL in the message.
// Any additional body text you'd like to add should be passed in the
// second argument.
+ (void)presentComposeEmailViewInViewController:(UIViewController *)parentViewController
                                       withBody:(NSString *)bodyText;


// Case 2: Use the following API for sharing if you want to use
//         your own custom UI and will also perform all of the 
//         Facebook/Twitter/YouTube authentication yourself.
//         Simply call this one function that will upload the video
//         to Kamcord (and optionally YouTube). Once the video is successfully
//         uploaded, you'll get a callback to 
//
//         - (void)videoIsReadyToShare:(NSURL *)onlineVideoURL
//                           thumbnail:(NSURL *)onlineThumbnailURL
//                             message:(NSString *)message
//                                data:(NSDictionary *)data
//                               error:(NSError *)error;
//
//         (defined above in KCShareDelegate).
//         If you don't want to upload to YouTube, simply pass
//         in nil for the youTubeAuth object.
//
//         The data object you pass in will be passed back to you
//         in videoIsReadyToShare.
//
//         Returns YES if the share was accepted for processing.
//         Returns NO if there was a previous share that is still
//         in its early stages (specifically, before a generalError:
//         or shareshareStartedWithSuccess:error: callback).
+ (BOOL)shareVideoWithMessage:(NSString *)message
              withYouTubeAuth:(GTMOAuth2Authentication *)youTubeAuth
                         data:(NSDictionary *)data;




+ (BOOL)handleOpenURL:(NSURL *)url;

// --------------------------------------------------------
// For Kamcord internal use, don't worry about these.

// Returns the singleton Kamcord object. You don't ever really need this, just
// use the static API calls.
+ (Kamcord *) sharedManager;

// Helper to figure calculate the internal scale factor
+ (unsigned int) resolutionScaleFactor;

+ (KCAudio *) audioBackground;

@end
