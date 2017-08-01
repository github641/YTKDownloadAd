//
//  YTKNetworkPrivate.h
//
//  Copyright (c) 2012-2016 YTKNetwork https://github.com/yuantiku
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#import <Foundation/Foundation.h>
#import "YTKRequest.h"
#import "YTKBaseRequest.h"
#import "YTKBatchRequest.h"
#import "YTKChainRequest.h"
#import "YTKNetworkAgent.h"
#import "YTKNetworkConfig.h"

NS_ASSUME_NONNULL_BEGIN

/* lzy注170718：
 本类库的日志打印装用log语句的对外声明
 */
FOUNDATION_EXPORT void YTKLog(NSString *format, ...) NS_FORMAT_FUNCTION(1,2);

@class AFHTTPSessionManager;

@interface YTKNetworkUtils : NSObject

/* lzy注170718：
 使用开发者指定的『客户端与服务端商定的各字段数据类型』说明，来检查返回JSON数据是否合规。
 */
+ (BOOL)validateJSON:(id)json withValidator:(id)jsonValidator;
/* lzy注170718：
 给沙盒中的某路径添加不备份属性标识
 */
+ (void)addDoNotBackupAttribute:(NSString *)path;
// 对字符串进行 md5 加密
+ (NSString *)md5StringFromString:(NSString *)string;
// 获取 app 的版本号
+ (NSString *)appVersionString;

+ (NSStringEncoding)stringEncodingWithRequest:(YTKBaseRequest *)request;
/* lzy注170718：
 检查resumeData的有效性。
 */
+ (BOOL)validateResumeData:(NSData *)data;

@end

/* lzy注170718：
 将一些YTKRequest.m的中的实例方法的声明，放在YTKNetworkPrivate.h类中，而不是放在YTKRequest.h中。
 从而如果不导入这个文件，不看源码，无从直到YTKRequest类可以实现了某方法，并可以响应。达到了一定的隐藏目的。
 下面同理。
 */
@interface YTKRequest (Getter)

- (NSString *)cacheBasePath;

@end

@interface YTKBaseRequest (Setter)

@property (nonatomic, strong, readwrite) NSURLSessionTask *requestTask;
@property (nonatomic, strong, readwrite, nullable) NSData *responseData;
@property (nonatomic, strong, readwrite, nullable) id responseJSONObject;
@property (nonatomic, strong, readwrite, nullable) id responseObject;
@property (nonatomic, strong, readwrite, nullable) NSString *responseString;
@property (nonatomic, strong, readwrite, nullable) NSError *error;

@end

@interface YTKBaseRequest (RequestAccessory)

- (void)toggleAccessoriesWillStartCallBack;
- (void)toggleAccessoriesWillStopCallBack;
- (void)toggleAccessoriesDidStopCallBack;

@end

@interface YTKBatchRequest (RequestAccessory)

- (void)toggleAccessoriesWillStartCallBack;
- (void)toggleAccessoriesWillStopCallBack;
- (void)toggleAccessoriesDidStopCallBack;

@end

@interface YTKChainRequest (RequestAccessory)

- (void)toggleAccessoriesWillStartCallBack;
- (void)toggleAccessoriesWillStopCallBack;
- (void)toggleAccessoriesDidStopCallBack;

@end

@interface YTKNetworkAgent (Private)

- (AFHTTPSessionManager *)manager;
- (void)resetURLSessionManager;
- (void)resetURLSessionManagerWithConfiguration:(NSURLSessionConfiguration *)configuration;

- (NSString *)incompleteDownloadTempCacheFolder;

@end

NS_ASSUME_NONNULL_END

