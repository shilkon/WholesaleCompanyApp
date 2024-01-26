#ifndef TABLE_H
#define TABLE_H

#include "queries.h"

#include <pqxx/pqxx>

#include <vector>
#include <tuple>
#include <memory>
#include <string>

namespace database
{
    namespace detail
    {
        template <typename... T>
        class Table
        {
        public:
            using RowType          = std::tuple<int, T...>;
            using RowTypeWithoutId = std::tuple<T...>;

            Table(pqxx::connection& con, const TableQueries& tableQueries);

            void updateData();

            const std::vector<RowType>& getData() const;
            RowType getById(int id) const;

            template <typename CT>
            RowType getByColumnValue(int cNumber, CT value) const;
            
            std::string create(T... args);
            std::string modify(int id, T... args);
            std::string deleteById(int id);

            std::string_view getTableName() const;

            void setDataNotMoified();
            bool isDataModified() const;

        private:
            pqxx::connection* con_;
            std::unique_ptr<TableQueries> tableQueries_;
            std::vector<RowType> data_{};
            bool dataModified_{};
        };

        template <typename... T>
        inline Table<T...>::Table(pqxx::connection& con, const TableQueries& tableQueries) :
            con_(&con),
            tableQueries_(std::make_unique<TableQueries>(tableQueries))
        {
            updateData();
        }

        template <typename... T>
        inline void Table<T...>::updateData()
        {
            data_.clear();

            pqxx::work tx{ *con_ };
            for (const auto& row : tx.query<int, T...>(tableQueries_->updateQuery))
                data_.push_back(row);
            tx.commit();

            dataModified_ = true;
        }

        template <typename... T>
        inline const std::vector<typename Table<T...>::RowType>& Table<T...>::getData() const
        {
            return data_;
        }

        template <typename... T>
        inline typename Table<T...>::RowType Table<T...>::getById(int id) const
        {
            return getByColumnValue(0, id);
        }

        template <typename ...T>
        template <typename CT>
        inline typename Table<T...>::RowType database::detail::Table<T...>::getByColumnValue(int cNumber, CT value) const
        {
            RowType row{};

            if (cNumber >= 0 && cNumber < tableQueries_->selectByColumnValueQueries.size())
            {
                pqxx::work tx{ *con_ };
                auto result{ tx.exec_params(tableQueries_->selectByColumnValueQueries[cNumber], value) };
                tx.commit();

                if (result.size())
                    result.front().to(row);
            }

            return row;
        }

        template <typename... T>
        inline std::string Table<T...>::create(T... args)
        {
            std::string resultStatus{};

            pqxx::work tx{ *con_ };
            try
            {
                tx.exec_params0(tableQueries_->createQuery, args...);
            }
            catch (const pqxx::sql_error& e)
            {
                resultStatus = e.what();
            }
            tx.commit();

            if (resultStatus.empty())
                updateData();

            return resultStatus;
        }

        template <typename... T>
        inline std::string Table<T...>::modify(int id, T... args)
        {
            std::string resultStatus{};

            pqxx::work tx{ *con_ };
            try
            {
                tx.exec_params0(tableQueries_->modifyQuery, id, args...);
            }
            catch (const pqxx::sql_error& e)
            {
                resultStatus = e.what();
            }
            tx.commit();

            if (resultStatus.empty())
                updateData();

            return resultStatus;
        }

        template <typename... T>
        inline std::string Table<T...>::deleteById(int id)
        {
            std::string resultStatus{};

            pqxx::work tx{ *con_ };
            try
            {
                tx.exec_params0(tableQueries_->deleteByIdQuery, id);
            }
            catch (const pqxx::sql_error& e)
            {
                resultStatus = e.what();
            }
            tx.commit();

            if (resultStatus.empty())
                updateData();

            return resultStatus;
        }

        template <typename... T>
        inline std::string_view Table<T...>::getTableName() const
        {
            return tableQueries_->tableName;
        }

        template <typename ...T>
        inline void Table<T...>::setDataNotMoified()
        {
            dataModified_ = false;
        }

        template <typename ...T>
        inline bool Table<T...>::isDataModified() const
        {
            return dataModified_;
        }
    }

    using GoodsTable              = detail::Table<std::string, int>;
    using SalesTable              = detail::Table<int, int, std::string>;
    using WarehouseTable          = detail::Table<int, int>;
    using PopularGoodsOfMonthView = detail::Table<std::string, int, int>;

    using UConnection              = std::unique_ptr<pqxx::connection>;
    using UGoodsTable              = std::unique_ptr<GoodsTable>;
    using USalesTable              = std::unique_ptr<SalesTable>;
    using UWarehouseTable          = std::unique_ptr<WarehouseTable>;
    using UPopularGoodsOfMonthView = std::unique_ptr<PopularGoodsOfMonthView>;
}

#endif
