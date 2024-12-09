#include "../inc/bee_user.h"

const char* uses_chat = NULL;
GtkWidget *chat_entry;

void on_add_message_button_clicked(GtkWidget *button, gpointer user_data) {
    (void)button;
    GtkEntry *entry = GTK_ENTRY(user_data);
    
    if (uses_chat == NULL) {
        mx_printerr("Chat not chosen\n");
        gtk_entry_set_text(entry, "");

        GtkWidget *dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "Error");
        gtk_widget_set_name(dialog, "dialog-search-person");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(entry))));

        GtkWidget *error = gtk_label_new("Chat not chosen");
        gtk_widget_set_name(error, "error-label");

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_container_add(GTK_CONTAINER(content_area), error);
        gtk_widget_show_all(dialog);
        g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
    } else {
        const char *message = gtk_entry_get_text(entry);
        if (message == NULL || mx_strlen(message) == 0) {
            mx_printerr("No message entered\n");
            gtk_entry_set_text(entry, "");
        } else {
            printf("Text entered: %s\n", message);
            gtk_entry_set_text(entry, "");
        }
    }
}

gboolean check_connection(gpointer user_data) {
    t_main *main_data = (t_main *)user_data;
    if (!main_data->is_connected) {
        GtkWidget *main_window = g_object_get_data(G_OBJECT(main_data->buff), "main-window");
        if (main_window) {
            gtk_widget_destroy(main_window);
        }

        login_window(main_data->app, main_data);

        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}

gboolean gtk_create_main_window(gpointer user_data) {
    (void)user_data;
    GtkWidget *main_window;
    GtkWidget *main_box;

    /* Вертикальный контейнер для topbar и остального */
    GtkWidget *vertical_box;

    /* Боковая панель */
    GtkWidget *sidebar_box;

    /* Контейнер чата */
    GtkWidget *chat_box;

    /* Верхняя панель чата */
    GtkWidget *topbar_box;
    GtkWidget *avatar_image;
    GdkPixbuf *pixbuf, *scaled_pixbuf;

    /* Поле ввода сообщений */
    GtkWidget *chat_entry_box;
   //GtkWidget *chat_entry;

    /* Контейнер для отображения сообщений */
    GtkWidget *message_area;

    /* Создание главного окна */
    main_window = gtk_application_window_new(GTK_APPLICATION(main_data->app));
    gtk_window_set_title(GTK_WINDOW(main_window), "BEE CHAT");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1280, 800);
    gtk_window_set_resizable(GTK_WINDOW(main_window), FALSE); // Фиксированный размер окна

    GtkStyleContext *main_context = gtk_widget_get_style_context(main_window);
    gtk_style_context_add_class(main_context, "main-window");

    /* Основной вертикальный контейнер */
    vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), vertical_box);

    /* Верхняя панель чата (topbar_box) */
    topbar_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(topbar_box, "top_bar");
    gtk_widget_set_size_request(topbar_box, -1, 60);
    gtk_box_pack_start(GTK_BOX(vertical_box), topbar_box, FALSE, FALSE, 0);

    /* Основной горизонтальный контейнер */
    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vertical_box), main_box, TRUE, TRUE, 0);

    /* Боковая панель (sidebar_box) */
    GtkWidget *sidebar_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(sidebar_scrolled, 300, -1); // Фиксированная ширина
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sidebar_scrolled),
                                GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC); // Политика прокрутки
    gtk_box_pack_start(GTK_BOX(main_box), sidebar_scrolled, FALSE, TRUE, 0);

    sidebar_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(sidebar_scrolled), sidebar_box);
    GtkStyleContext *sidebar_context = gtk_widget_get_style_context(sidebar_box);
    gtk_style_context_add_class(sidebar_context, "side-box"); // Добавляем класс для CSS

    /* Добавление аватарки в боковую панель */
    pixbuf = gdk_pixbuf_new_from_file("img/avatar.png", NULL);
    scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, 60, 60, GDK_INTERP_BILINEAR); // Масштабирование до 60x60
    avatar_image = gtk_image_new_from_pixbuf(scaled_pixbuf);

    g_object_unref(pixbuf); // Освобождаем память оригинального изображения
    g_object_unref(scaled_pixbuf); // Освобождаем память масштабированного изображения

    gtk_widget_set_halign(avatar_image, GTK_ALIGN_START); // Выравнивание по левому краю
    gtk_box_pack_start(GTK_BOX(topbar_box), avatar_image, FALSE, FALSE, 0); // Добавление сверху

    /* Контейнер для чата */
    chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_box), chat_box, TRUE, TRUE, 0);
    GtkStyleContext *chat_context = gtk_widget_get_style_context(chat_box);
    gtk_style_context_add_class(chat_context, "chat-box"); // Добавляем класс для CSS

   /* Контейнер для отображения сообщений (область истории чата) */
    GtkWidget *message_scrolled = gtk_scrolled_window_new(NULL, NULL);
    message_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(message_scrolled, TRUE); // Позволяет растягиваться по вертикали
    gtk_widget_set_hexpand(message_scrolled, TRUE); // Позволяет растягиваться по горизонтали
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(message_scrolled),
                                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); // Политика прокрутки
    gtk_box_pack_start(GTK_BOX(chat_box), message_scrolled, TRUE, TRUE, 0);

    message_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(message_scrolled), message_area);
    GtkStyleContext *message_area_context = gtk_widget_get_style_context(message_area);
    gtk_style_context_add_class(message_area_context, "message-area"); // Добавляем CSS-класс


    /* Контейнер для ввода сообщений */
    chat_entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(chat_entry_box, "chat_entry_box");
    gtk_box_pack_end(GTK_BOX(chat_box), chat_entry_box, FALSE, FALSE, 0);
    //gtk_widget_set_halign(chat_entry_box, GTK_ALIGN_CENTER);

    // Кнопка создания чата
    GtkWidget *button_create_chat = gtk_button_new();
    GtkWidget *image_create_chat = create_image("img/addrofa.svg", 20, 15);
    gtk_button_set_image(GTK_BUTTON(button_create_chat), image_create_chat);
    gtk_box_pack_start(GTK_BOX(chat_entry_box), button_create_chat, FALSE, FALSE, 0);
    gtk_widget_set_name(button_create_chat, "create-chat-button");

    // Подключаем обработчик для кнопки
    g_signal_connect(button_create_chat, "clicked", G_CALLBACK(on_button_create_chat_clicked), sidebar_box);

    // кнопка передачі фото
    GtkWidget *file_button = gtk_button_new();
    GtkWidget *image_load = create_image("img/uploadrofa.svg", 20, 15);
    gtk_button_set_image(GTK_BUTTON(file_button), image_load);
    gtk_box_pack_start(GTK_BOX(chat_entry_box), file_button, FALSE, FALSE, 0);
    gtk_widget_set_name(file_button, "create-chat-button");
    gtk_widget_set_margin_start(file_button, 30);
    g_signal_connect(file_button, "clicked", G_CALLBACK(on_file_button_clicked), main_window);

    // кнопка передачі смайликів
    GtkWidget *emoji_button = gtk_button_new_with_label("😊");
    gtk_box_pack_start(GTK_BOX(chat_entry_box), emoji_button, FALSE, FALSE, 0);
    gtk_widget_set_name(emoji_button, "create-chat-button");
    gtk_widget_set_margin_start(emoji_button, 10);
    g_signal_connect(emoji_button, "clicked", G_CALLBACK(on_emoji_button_clicked), emoji_button);

    /* Поле для ввода сообщений */
    chat_entry = gtk_entry_new();
    gtk_widget_set_name(chat_entry, "chat_entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(chat_entry), "Type a message...");
    gtk_widget_set_size_request(chat_entry, 500, 40);
    gtk_widget_set_halign(chat_entry, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(chat_entry_box), chat_entry, FALSE, FALSE, 0);
    gtk_widget_set_margin_start(chat_entry, 150);

   
    // кнопка для відсилання повідомленння
    GtkWidget *send_button = gtk_button_new();
    GtkWidget *image_send_button = create_image("img/sendrofa.svg", 20, 15);
    gtk_button_set_image(GTK_BUTTON(send_button), image_send_button);
    gtk_box_pack_start(GTK_BOX(chat_entry_box), send_button, FALSE, FALSE, 0);
    gtk_widget_set_name(send_button, "send-button");
    gtk_widget_set_margin_start(send_button, 50);
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_add_message_button_clicked), chat_entry);

    /* Сохранение указателя на главное окно */
    g_object_set_data(G_OBJECT(main_data->buff), "main-window", main_window);

    // Запуск проверки соединения каждые 1000 мс
    g_timeout_add(1000, check_connection, main_data);

    /* Отображение всех виджетов */
    gtk_widget_show_all(main_window);

    return G_SOURCE_REMOVE;
}
