package com._4paradigm.fesql_auto_test.v1;

import com._4paradigm.fesql.sqlcase.model.SQLCase;
import com._4paradigm.fesql_auto_test.common.FesqlTest;
import com._4paradigm.fesql_auto_test.entity.FesqlDataProvider;
import com._4paradigm.fesql_auto_test.executor.ExecutorFactory;
import lombok.extern.slf4j.Slf4j;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import java.io.FileNotFoundException;

/**
 * @author zhaowei
 * @date 2020/6/11 2:53 PM
 */
@Slf4j
public class SelectTest extends FesqlTest {

    @DataProvider
    public Object[] testSampleSelectData() throws FileNotFoundException {
        FesqlDataProvider dp = FesqlDataProvider
                .dataProviderGenerator("/integration/v1/test_select_sample.yaml");
        return dp.getCases();
    }

    @Test(dataProvider = "testSampleSelectData")
    public void testSampleSelect(SQLCase testCase) throws Exception {
        ExecutorFactory.build(executor,testCase).run();
    }
    @DataProvider
    public Object[] testExpressionData() throws FileNotFoundException {
        FesqlDataProvider dp = FesqlDataProvider
                .dataProviderGenerator("/integration/v1/test_expression.yaml");
        return dp.getCases();
    }

    @Test(enabled = false, dataProvider = "testExpressionData")
    public void testExpression(SQLCase testCase) throws Exception {
        ExecutorFactory.build(executor,testCase).run();
    }
    @DataProvider
    public Object[] testUDAFFunctionData() throws FileNotFoundException {
        FesqlDataProvider dp = FesqlDataProvider
                .dataProviderGenerator("/integration/v1/test_udaf_function.yaml");
        return dp.getCases();
    }

    @Test(enabled = false, dataProvider = "testUDAFFunctionData")
    public void testUDAFFunction(SQLCase testCase) throws Exception {
        ExecutorFactory.build(executor,testCase).run();
    }
    @DataProvider
    public Object[] testSubSelectData() throws FileNotFoundException {
        FesqlDataProvider dp = FesqlDataProvider
                .dataProviderGenerator("/integration/v1/test_sub_select.yaml");
        return dp.getCases();
    }

    @Test(enabled = false, dataProvider = "testSubSelectData")
    public void testSubSelect(SQLCase testCase) throws Exception {
        ExecutorFactory.build(executor,testCase).run();
    }
}
