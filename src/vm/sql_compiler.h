/*
 * Copyright 2021 4Paradigm
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_VM_SQL_COMPILER_H_
#define SRC_VM_SQL_COMPILER_H_

#include <memory>
#include <set>
#include <string>
#include <vector>
#include "base/fe_status.h"
#include "llvm/IR/Module.h"
#include "parser/parser.h"
#include "proto/fe_common.pb.h"
#include "udf/udf_library.h"
#include "vm/catalog.h"
#include "vm/engine_context.h"
#include "vm/jit_wrapper.h"
#include "vm/runner.h"

namespace fesql {
namespace vm {

using fesql::base::Status;

struct BatchRequestInfo {
    // common column indices in batch request mode
    std::set<size_t> common_column_indices;

    // common physical node ids during batch request
    std::set<size_t> common_node_set;

    // common output column indices
    std::set<size_t> output_common_column_indices;
};

struct SQLContext {
    // mode: batch|request|batch request
    ::fesql::vm::EngineMode engine_mode;
    bool is_performance_sensitive = false;
    bool is_cluster_optimized = false;
    bool is_batch_request_optimized = false;
    bool enable_expr_optimize = false;
    bool enable_batch_window_parallelization = false;

    // the sql content
    std::string sql;
    // the database
    std::string db;
    // the logical plan
    ::fesql::node::PlanNodeList logical_plan;
    ::fesql::vm::PhysicalOpNode* physical_plan = nullptr;
    fesql::vm::ClusterJob cluster_job;
    // TODO(wangtaize) add a light jit engine
    // eg using bthead to compile ir
    fesql::vm::JITOptions jit_options;
    std::shared_ptr<fesql::vm::FeSQLJITWrapper> jit = nullptr;
    Schema schema;
    Schema request_schema;
    std::string request_name;
    uint32_t row_size;
    std::string ir;
    std::string logical_plan_str;
    std::string physical_plan_str;
    std::string encoded_schema;
    std::string encoded_request_schema;
    ::fesql::node::NodeManager nm;
    ::fesql::udf::UDFLibrary* udf_library = nullptr;

    ::fesql::vm::BatchRequestInfo batch_request_info;

    SQLContext() {}
    ~SQLContext() {}
};

bool RegisterFeLibs(udf::UDFLibrary* lib, base::Status& status,  // NOLINT
                    const std::string& libs_home = "",
                    const std::string& libs_name = "");
bool GetLibsFiles(const std::string& dir_path,
                  std::vector<std::string>& filenames,  // NOLINT
                  base::Status& status);                // NOLINT
const std::string FindFesqlDirPath();

class SQLCompileInfo : public CompileInfo {
 public:
    SQLCompileInfo() {}
    ~SQLCompileInfo() {}
    fesql::vm::SQLContext& get_sql_context() { return this->sql_ctx; }

    bool get_ir_buffer(const base::RawBuffer& buf) {
        auto& str = this->sql_ctx.ir;
        return buf.CopyFrom(str.data(), str.size());
    }
    size_t get_ir_size() { return this->sql_ctx.ir.size(); }

    const fesql::vm::Schema& GetSchema() const { return sql_ctx.schema; }

    const fesql::vm::ComileType GetCompileType() const {
        return ComileType::kCompileSQL;
    }
    const fesql::vm::EngineMode GetEngineMode() const {
        return sql_ctx.engine_mode;
    }
    const std::string& GetEncodedSchema() const {
        return sql_ctx.encoded_schema;
    }

    virtual const Schema& GetRequestSchema() const {
        return sql_ctx.request_schema;
    }
    virtual const std::string& GetRequestName() const {
        return sql_ctx.request_name;
    }

    fesql::vm::PhysicalOpNode* GetPhysicalPlan() {
        return sql_ctx.physical_plan;
    }
    virtual fesql::vm::Runner* GetMainTask() {
        return sql_ctx.cluster_job.GetMainTask().GetRoot();
    }
    virtual fesql::vm::ClusterJob& GetClusterJob() {
        return sql_ctx.cluster_job;
    }
    virtual void DumpPhysicalPlan(std::ostream& output,
                                  const std::string& tab) {
        sql_ctx.physical_plan->Print(output, tab);
    }
    virtual void DumpClusterJob(std::ostream& output, const std::string& tab) {
        sql_ctx.cluster_job.Print(output, tab);
    }
    static SQLCompileInfo *CastFrom(CompileInfo *node) {
        return dynamic_cast<SQLCompileInfo*>(node);
    }
 private:
    fesql::vm::SQLContext sql_ctx;
};

class SQLCompiler {
 public:
    SQLCompiler(const std::shared_ptr<Catalog>& cl, bool keep_ir = false,
                bool dump_plan = false, bool plan_only = false);

    ~SQLCompiler();

    bool Compile(SQLContext& ctx,                 // NOLINT
                 Status& status);                 // NOLINT
    bool Parse(SQLContext& ctx, Status& status);  // NOLINT
    bool BuildClusterJob(SQLContext& ctx,         // NOLINT
                         Status& status);         // NOLINT

 private:
    void KeepIR(SQLContext& ctx, llvm::Module* m);  // NOLINT

    bool ResolvePlanFnAddress(PhysicalOpNode* node,
                              std::shared_ptr<FeSQLJITWrapper>& jit,  // NOLINT
                              Status& status);                        // NOLINT

    Status BuildPhysicalPlan(SQLContext* ctx,
                             const ::fesql::node::PlanNodeList& plan_list,
                             ::llvm::Module* llvm_module,
                             PhysicalOpNode** output);
    Status BuildBatchModePhysicalPlan(
        SQLContext* ctx, const ::fesql::node::PlanNodeList& plan_list,
        ::llvm::Module* llvm_module, udf::UDFLibrary* library,
        PhysicalOpNode** output);
    Status BuildRequestModePhysicalPlan(
        SQLContext* ctx, const ::fesql::node::PlanNodeList& plan_list,
        ::llvm::Module* llvm_module, udf::UDFLibrary* library,
        PhysicalOpNode** output);
    Status BuildBatchRequestModePhysicalPlan(
        SQLContext* ctx, const ::fesql::node::PlanNodeList& plan_list,
        ::llvm::Module* llvm_module, udf::UDFLibrary* library,
        PhysicalOpNode** output);

 private:
    const std::shared_ptr<Catalog> cl_;
    bool keep_ir_;
    bool dump_plan_;
    bool plan_only_;
};

}  // namespace vm
}  // namespace fesql
#endif  // SRC_VM_SQL_COMPILER_H_
